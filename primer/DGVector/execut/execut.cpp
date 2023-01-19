#include "execut.h"
#include <QRegExp>
//#include <QApplication>

#ifndef _WIN_
	uint execut::comDelay = 5;//принудительная задержка отправки
	uint execut::comMWait  = 15;//макс разрыв
#else
	uint execut::comDelay = 15;//принудительная задержка отправки
	uint execut::comMWait  = 50;//макс разрыв
#endif



execut::execut(uchar deb,QObject *parent): QObject(parent){
	//режим теста
	qRegisterMetaType<command*>("command*");
	debug=(deb>0);
	mdebug=false;
	dadr=deb;
	tdebug=false;
	couTD=0;
	couT=0;
	sumT=0;
	endTD=10;

    gcou=-1;
    run=false;
    Tape=&TapeI;
    extExe=NULL;
    extCOM=NULL;
    com=NULL;
    socket=NULL;
    usocket=NULL;
    curCMD=NULL;
    time=NULL;
    after_delay=0;
    Port=0;
    RepeatCoun=tcpERRCoun=0;
    taut=0;          //таймер таимаута ожидания данных
    tautS=0;
    tautM=0;
    pendT=0;
    act_time=0;
    idtran.ushor=0;
   connect(this, SIGNAL(pendTime(bool)), this ,SLOT(runTime(bool)));

}

execut::~execut(){
	if(mdebug) qDebug()<<"destruct exe";
	if(run) Stop();
	delete(time);
	Tape->clear();
	if(Tape!=&TapeI){
		if(mdebug) qDebug()<<"del tap";
		delete(Tape);
	}

}


void execut::runTime(bool reset){
	int delay=0;
	if(reset){
		if(pendT)  killTimer(pendT);
		if(transp==COM)	delay=comDelay;
		pendT=startTimer(delay);
	}else if(pendT==0) pendT=startTimer(delay);
}


bool execut::Start(){
	if(Tape->size()==0)	    return false;
	if(!Tape->test(debug))	return false;
	gsend.reserve(254);     //массив запроса (без суфиксов и префиксов)
	rsend.reserve(260);     //массив запроса (с суфиксами и префиксами)
	resp.reserve(260*2);      //ответ
	gresp.reserve(254);
	RepeatCoun=tcpERRCoun=0;
	after_delay=0;
	if(time==NULL) time=new QElapsedTimer();
	run=true;
	if(mdebug) qDebug()<<"start"<<Tape->marker;
	emit statScharman(true);
	Next(0);
	return true;
}

void execut::Stop(bool del){
	if(transp== EXT && run){  //можно закрыть только по возврату
		run=false;
		if(extExe->getStatus()){
			if(mdebug) qDebug()<<"no stop EXT run"<<Tape->marker;
			return;
		}
	}

	run=false;

	after_delay=0;
	if(transp!= NONE) disconnect_io();
	gcou=-1;
	curCMD=NULL;
	if(mdebug) qDebug()<<"stop"<<Tape->marker;
	emit statScharman(false);
	emit finish(Tape->marker);
	if(del) deleteLater();

}

/*
void execut::stop(int m){

	run=false;
	after_delay=0;
	if(transp!= NONE) disconnect_io();
	gcou=-1;
	curCMD=NULL;
	if(mdebug) qDebug()<<"stop for parent"<<m<<"->"<<Tape->marker;
	emit statScharman(false);
	emit finish(Tape->marker);
}
*/


void execut::Next(int poz){

	if(poz<0 || poz>=Tape->size() || !run){
		run=false;
		Stop();
		return;
	}
	if(gcou!=poz) emit statScharman(poz);
	gcou=poz;
	if(act_time>0){ //корректировка времени - если засекали - обеспечиваем постоянное время цикла
        //отдадка по времени опроса - обновления данных
		if(tdebug && endTD>0){
			if(couTD==0){
				couTD=4000/qMax(after_delay,act_time);
				if(couTD==0) couTD=1;
			}
			sumT+=act_time;
			if(++couT==couTD){
				 uchar ad;
				 if(Tape->at(gcou)->Type>=IF) ad=((dIF*)Tape->at(gcou))->adr();
				 emit timeCycle(ad,sumT/couT,after_delay);
				 //qDebug()<<QString::number(ad)+":cycle time:"+QString::number(sumT/couT)+"/"+QString::number(after_delay);
				 couT=0;  sumT=0; endTD--;

		    }
		}
		//----------------------------------------------------------------------
		//задержка после - это время цикла опроса - из него вычитаем в ремя запроса.
		after_delay = after_delay>act_time ? after_delay-act_time : 0;
		act_time=0;
	}
	tautS=startTimer(Tape->at(gcou)->delay+after_delay);
}


void execut::timerEvent(QTimerEvent *event)
{
     int id=event->timerId();

     command* CMD;
	 if(id==tautM){ //мультикаст
		    dMULTICAST* dm;
		   	dm=(dMULTICAST*)curCMD;
			udpAddrSen.setAddress(dm->getNextAdr());
			Send();
			if(dm->getRez()==OK){
				killTimer(tautM);
				tautM=0;
				taut=startTimer(curCMD->wait);//будем ждать прихода всех ответов
			}
			return;
	 }
	 if(id==taut){ //время ожидания истекло
	    	killTimer(taut);
	    	taut=0;
	    	CMD=curCMD;
	    	curCMD=NULL;
	       	ttaut(CMD);
	       	if(transp==TCP){
	       		if( CMD->getType()!=BROADCAST && CMD->getType()!=MULTICAST){
					 tcpERRCoun++;
					 if(tcpERRCoun>Tape->getMaxTcpERRCoun()){
						 emit messageError("Maximum TCP erorr "+socket->peerAddress().toString()+":"+QString::number(socket->peerPort ()));
						 Stop();
						 return;
					 }
	       		}
	        }
	}else if(id==tautS){  //заталкивает в очередь команду текущей шарманки
    	killTimer(tautS);
    	tautS=0;
    	mutex.lock();
    	queueCMD.enqueue(Tape->at(gcou));
    	mutex.unlock();
   }else if(id==pendT){ //пришла команда в очередь извне или обработана предыдущая текщей шарманки
	    killTimer(id);
	    pendT=0;
   }


  if(run && !queueCMD.isEmpty() && curCMD==NULL && pendT==0){
	  mutex.lock();
	  CMD=queueCMD.dequeue();
	  mutex.unlock();
	  SScharmanka(CMD);
  }

}



/**
 * Отправляем
 */
void execut::SScharmanka(command* CMD){

	//готовим посылку
	if(debug){
	   QString txt;
	   if(transp!=EXT){
		   switch(CMD->Retrans){
		   	   case NotRet:
		   		   qDebug()<<"ST"+txt.setNum(gcou,10)+(gsend.size()>0 ? (":"+QString::number((uchar)gsend[0])) : "");
		   		   break;
		   	   case ExternRet:
		   		   qDebug()<<"BR:"+(gsend.size()>0 ? QString::number((uchar)gsend[0]) : "");
		   		   break;
		   	   case NoBlockRet:
		   		   qDebug()<<"NR:"+(gsend.size()>0 ? QString::number((uchar)gsend[0]) : "");
		   		   break;
		   }
	   }
	}

	CMD->setERR(NOERR);
	curCMD=CMD;
	curWait=CMD->wait;
	gsend.clear();

	switch(CMD->getType()){
		case IF:
		case SETTIME:
		case FOR:
		case DOWNLOAD:
		case MULTICAST:
		case BROADCAST:
			if(transp==NONE){
					CMD->setRez(BAD);
					taut=startTimer(curWait);
					return;
			}
			gsend.append(CMD->getRES());
			break;
		case CONNECT:
			dCONNECT*  dco;
			dco=(dCONNECT*)CMD;
			disconnect_io();//если была связь - рвем
			connect_io(dco);
			break;
		case DISCONNECT:
			disconnect_io();
			CMD->setRez(OK);
			break;
		case NOOP:
			CMD->setRez(OK);
			break;
	}


	if(gsend.size()>0){//есть посылка
			if(CMD->getType()==MULTICAST){
				if(udpAddrRes!=QHostAddress(QHostAddress::Broadcast)) udpAddrRes=QHostAddress(QHostAddress::Broadcast);
				tautM=startTimer(CMD->delay);
				return;
			}else{
				if(CMD->getType()==BROADCAST) udpAddrSen=QHostAddress(QHostAddress::Broadcast);
				if(Send()) return;
				else       CMD->setRez(BAD);
			}
	}


	//Это внешняя команда?
	if(CMD->Retrans!=NotRet) return;

	//далее только внутренняя
	curCMD=NULL;

	//sendMess(CMD,CMD->rez);
	if(CMD->ControlRez) emit rezult(CMD);
	//runCFunc(CMD,CMD->rez,udpAddrSen.toIPv4Address(),QByteArray());



	//переход на следующую
	switch(CMD->getRez()){
		case OK:
			after_delay=CMD->after_delay_OK;
			Next(CMD->getOK());
			break;
		case BAD:
			after_delay=CMD->after_delay_BAD;
			Next(CMD->getBAD());
			break;
		default:
			break;
	}

}


/*Слот
 * Анализирует ответы терминала
 */
void execut::Receive(const QByteArray& arr){

	if(debug){
		if(transp!=EXT){
			if(arr.size()>0 && (uchar)arr[0]==dadr){
				qDebug()<<"<-"+ByteArray_to_String(arr);
			}
		}else qDebug()<<"Ext receive:"<<arr.size();
	}
	if(!curCMD) return;
	command* CMD=curCMD;
	//QByteArray rez;

	if(arr.size()==3){ //проверяем на ошибку шлюза
		switch(CMD->getType()){
			case IF:
			case SETTIME:
			case FOR:
			case DOWNLOAD:
				//
				uchar err;
				if(((dIF*)CMD)->fun()+0x80==(uchar)arr[1]){
					err=(uchar)arr[2];
			        switch(err){
			             case 0x04: CMD->setERR(GWTIME);//ошибка слейва
			            	 break;
			             case 0x06: CMD->setERR(GWBUSI);//ошибка шлюза-занято
			            	 break;
			             case 0x0b: CMD->setERR(GWNOSL);//слейв отсутствует
			            	 break;
			        }

				}
				if(debug) if(CMD->getERR()>=GWTIME)  qDebug()<<"<-errGW:"<<err;
				break;
			default:
				break;
		}
	}

	if(arr.size()==0) CMD->setERR(TIMEERR);

	if(CMD->getERR()!=NOERR){

		if(taut){
			killTimer(taut);
			taut=0;
		}
		curCMD=NULL;
		ttaut(CMD);
		return;
	}

	if(transp==EXT){
		if(!run){ //внешнее закрываем тут!!!
			Stop();
			return;
		}
	}

	 //только если ждали ответа
	if(CMD->getType()!=MULTICAST && CMD->getType()!=BROADCAST){

		if(transp==TCP) tcpERRCoun=0;
		if(taut){
		   killTimer(taut);
		   taut=0;
		}
		if(CMD->tCommand & STOP){
			   act_time=time->elapsed();
			   emit timeAct(act_time);
		}
		curCMD=NULL;
		emit pendTime(true); //запускаем след команду из очереди
	}
    RScharmanka(CMD,arr);
}

/**
 * Получаем
 */
void execut::RScharmanka(command* CMD,const QByteArray& rep){



	//проверка условий
	switch(CMD->getType()){
		case IF:             ((dIF*)CMD)->test(rep);        break;
		case SETTIME:        ((dSETTIME*)CMD)->test(rep);   break;
		case FOR:            ((dFOR*)CMD)->test(rep);       break;
		case DOWNLOAD: {
			((dDOWNLOAD*)CMD)->test(rep);
			if(((dDOWNLOAD*)CMD)->oldpr!=((dDOWNLOAD*)CMD)->proc){
				emit statDownload(((dDOWNLOAD*)CMD)->proc);
			}
			break;
		}
		case MULTICAST:      ((dMULTICAST*)CMD)->test(rep); break;
		case CONNECT:
		case DISCONNECT:
		case BROADCAST:
		break;
	}

	if(CMD->getType()==MULTICAST || CMD->getType()==BROADCAST) 	return;


	//Это команда из другой шарманки или внешняя?
	if(CMD->Retrans==NoBlockRet || CMD->Retrans==ExternRet ){
		extReceive(CMD,rep); return;
	}

	if(transp!=TCP) RepeatCoun=0;//сброс числа ошибок связи
	//обработка результатов
	//sendMess(CMD,CMD->getRez());
	if(CMD->ControlRez) emit rezult(CMD);
	//runCFunc(CMD,CMD->getRez(),udpAddrRF.toIPv4Address(),rep);


	switch(CMD->getRez()){
		case OK:
			after_delay=CMD->after_delay_OK;
			Next(CMD->getOK());
			break;
		case BAD:
		case ERRCON:
			after_delay=CMD->after_delay_BAD;
			Next(CMD->getBAD());
			break;
		case CON:
			after_delay=((dFOR*)CMD)->after_delay_CON;
			Next(gcou);
			break;
		case NOT:
			break;
	}

}

void execut::extReceive(command* CMD,const QByteArray& res){
	if(CMD->parentEXE==NULL) return;
	if(CMD->Retrans==NoBlockRet){
		((execut*)CMD->parentEXE)->extReceive(res);
	}else if(CMD->Retrans==ExternRet){
		QMetaObject::invokeMethod(CMD->parentEXE,"extReceive",Qt::DirectConnection,Q_ARG(command*,CMD));
	}
}

bool execut::Send(){

	if(!transp) return false;
	rsend.clear();
	switch(transp){
		case COM:
			rsend=gsend;//остальное в драйвере
			break;
		case TCP:
		case UDP:
			ucrc size;
			idtran.shor++;  //сквозная нумерация
			//идентификатор транзакции
			rsend.append(idtran.cha[1]);
			rsend.append(idtran.cha[0]);
			//идентификатор протокола
			rsend.append((char)0);
			rsend.append((char)0);

			//пишем длинну данных
			size.shor=(unsigned short)(gsend.size());
			rsend.append(size.cha[1]);
			rsend.append(size.cha[0]);
			//данные
			rsend.append(gsend);
			break;
		case EXT:  //шлем как есть
			rsend=gsend;
			break;
		case NONE:
			return false;
	}

   if(debug){
	   if(!udpAddrSen.isNull()) qDebug()<<"UDP to:"<<udpAddrSen.toString();
	   if(transp!=EXT && (uchar)gsend[0]==dadr) qDebug()<<"->"+ByteArray_to_String(gsend);
   }

   switch(curCMD->getType()){
   		case IF:
   		case BROADCAST:
   		case SETTIME:
   		case FOR:
   		case DOWNLOAD:
    		if(transp!=EXT) taut=startTimer(curWait);//будем ждать ответа
    		if(curCMD->tCommand & START){act_time=0; time->start();}
   			break;
   		case MULTICAST:
   		case CONNECT:
   		case DISCONNECT:
   		case NOOP:
   			break;
   }

   emit putCh(rsend);
   return true;
}

/*
 * Прерывание работы по таймауту
 */
void execut::ttaut(command* CMD){


	 if(CMD->Retrans==NoBlockRet || CMD->Retrans==ExternRet){
		 if(CMD->Type==IF){
			 if(transp==COM && com->getProt()==Vector){
				 com->closeVector();
				 runTime(true);//взводим задержку
			 }
			 CMD->setRez(BAD);

		 }
		 else   			CMD->setRez(OK);  //BROADCAST
		 extReceive(CMD,emptyArr);
		 return;
	 }


	 if(debug) qDebug()<<"timeaut>"<<CMD->wait;


	 if(CMD->getType()==MULTICAST){
		//dMULTICAST* dm;
		//dm=(dMULTICAST*)CMD;
		udpAddrRes=Addr;
		udpAddrSen=Addr;

		/*
		QList<quint32> key=dm->multiArray.keys();
		for(int i=0; i<key.size();i++){
			runCFunc(CMD,OK,key[i],dm->multiArray[key[i]]);
		}*/
		//dm->multiArray.clear();
		//sendMess(CMD,CMD->rez);

		if(CMD->ControlRez) emit rezult(CMD);

		after_delay=CMD->after_delay_OK;
		Next(CMD->getOK());
		return;
	 }

	 if(CMD->getType()==BROADCAST){
			udpAddrSen=Addr; //возвращаем параметр
			//sendMess(CMD,CMD->rez);
			if(CMD->ControlRez) emit rezult(CMD);
			after_delay=CMD->after_delay_OK;
			Next(CMD->getOK());
			return;
	 }


	 if(++RepeatCoun<Tape->getMaxRepeat() && CMD->getERR()!=GWNOSL){ //повторы для ненадежного соединения
		 if(debug) qDebug()<<"Repeat:"<<RepeatCoun;
		 CMD->setRez(CON);
		 Next(gcou);
	 }else{
		 if(debug) if(RepeatCoun>=Tape->getMaxRepeat()) qDebug()<<"Connection failures is exceeded!";
		 RepeatCoun=0;//сброс числа ошибок
		 CMD->setRez(ERRCON);
		 if(CMD->Type==DOWNLOAD) emit errorDownload(gcou);
		 if(CMD->ControlRez)     emit rezult(CMD);
		 after_delay=CMD->after_delay_BAD;
		 Next(CMD->getBAD());
	 }
}

void execut::disconnect_io(){
	if(transp) if(debug) qDebug()<<"disconnect"<<Tape->marker;

	mutex.lock();

	if(tautS) { killTimer(tautS); tautS=0;}
	if(taut)  { killTimer(taut ); taut =0;}
	if(tautM) { killTimer(tautM ); tautM =0;}
	if(pendT) { killTimer(pendT ); pendT =0;}

	switch(transp){
		case COM:
			comOpen(false);
			delete(com);
			com=NULL;
			break;
		case TCP:
			socket->disconnectFromHost();
			ipOpen(false);
			delete(socket);
			socket=NULL;
			break;
		case UDP:
			udpOpen(false);
			delete(usocket);
			usocket=NULL;
			break;
		case EXT:
			extOpen(false);
			delete(extCOM);
		case NONE: break;
	}
	transp=NONE;

	//чистим очередь
	command* CMD;

	while(!queueCMD.isEmpty()){
		CMD=queueCMD.dequeue();
		CMD->setRez(BAD);
		if(CMD->Retrans==NoBlockRet || CMD->Retrans==ExternRet){
			 extReceive(CMD,emptyArr);
			 continue;
		}

	}
	mutex.unlock();
}


void execut::connect_io(dCONNECT* dco){
	bool ok;

	mutex.lock();

	switch(dco->cType){//тип коннекта
		case cCOM:
			com = new com485(dco->cAddr,dco->cProt485);
            com->setDWait(comMWait);
			com->setBaudRate(dco->cParam.toInt());
			if(com->open(QIODevice::ReadWrite)){
				comOpen(true);
				transp=COM;
				comPort=dco->cAddr;
				dco->setRez(OK);
				if(debug) qDebug()<<"connect "+ comPort;
			}else{
				comOpen(false);
				delete com;
				dco->setRez(BAD);
			}
			break;
		case cTCP:
			socket = new  QTcpSocket(this);
			socket->connectToHost(dco->cAddr,dco->cParam.toInt(&ok,10),QIODevice::ReadWrite|QIODevice::Unbuffered);
			if(!socket->waitForConnected(3000)){
				ipOpen(false);
				socket->disconnectFromHost();
				delete socket;
				dco->setRez(BAD);
			}else{
				Addr.setAddress(dco->cAddr);
				Port=dco->cParam.toInt(&ok,10);
				transp=TCP;
				tcpERRCoun=0;
				ipOpen(true);
				dco->setRez(OK);
				if(debug) qDebug()<<"connect "+dco->cAddr;
			}
			break;
		case cUDP:
			if(udpSAddr.setAddress(dco->udpSAddr)){
				udpSPort=dco->udpSPort;
				usocket = new QUdpSocket(this);
				usocket->bind(udpSAddr,udpSPort);
				if(dco->cAddr!=""){
					Addr.setAddress(dco->cAddr);
					udpAddrRes=Addr;
					udpAddrSen=Addr;
				}
				if(dco->cParam!="") Port=dco->cParam.toInt(&ok,10);

				udpOpen(true);
				transp=UDP;
				dco->setRez(OK);
				if(debug) qDebug()<<"connect "+dco->cAddr;
			}else dco->setRez(BAD);
			break;
		case cEXT:
			if(dco->extExe!=NULL){
				extExe=dco->extExe;
				//при останове главной - тормозим дочерние
				//QObject::connect(extExe, SIGNAL(finish(int)), this, SLOT(stop(int)));
			}
			if(extExe==NULL)       dco->setRez(BAD);
			else{
				extCOM=new dIF();
				extCOM->parentEXE=this;
				extOpen(true);
				transp=EXT;
				dco->setRez(OK);
				if(debug) qDebug()<<"connect EXT"<<Tape->marker;
			}
			break;
	}
	mutex.unlock();

}

/*
 * COM
 */
void execut::comOpen(bool ck){

	if(ck){
		//из порта
		QObject::connect(com, SIGNAL(dataRead(const QByteArray&)), this, SLOT(Receive(const QByteArray&)));
		//посылка команд из данного класса в порт
		QObject::connect(this, SIGNAL(putCh(const QByteArray&)), com ,SLOT(dataWrite(const QByteArray&)));
		//сохранялки

		//вывод отладки
		QObject::connect(com, SIGNAL(errorStr(const QString&)), this ,SIGNAL(messageError(const QString&)));
		 emit Onlain(true);
	}	else  {
		//из порта
		QObject::disconnect(com, SIGNAL(dataRead(const QByteArray&)), this, SLOT(Receive(const QByteArray&)));
		//посылка команд из данного класса в порт
		QObject::disconnect(this, SIGNAL(putCh(const QByteArray&)), com ,SLOT(dataWrite(const QByteArray&)));

		//вывод отладки
		QObject::disconnect(com, SIGNAL(errorStr(const QString&)), this ,SLOT(messageError(const QString&)));
		emit Onlain(false);
	}
}

/*
 * IP
 */
void execut::ipOpen(bool ck){
	if(ck){
		emit Onlain(true);
		//из порта
		QObject::connect(socket, SIGNAL(readyRead()), this, SLOT(readIPSlot()));
		//посылка команд из данного класса в порт
		QObject::connect(this, SIGNAL(putCh(const QByteArray&)), this ,SLOT(putIPSlot(const QByteArray&)));
		QObject::connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));


	}else{
        emit Onlain(false);
		//из порта
		QObject::disconnect(socket, SIGNAL(readyRead()), this, SLOT(readIPSlot()));
		//посылка команд из данного класса в порт
		QObject::disconnect(this, SIGNAL(putCh(const QByteArray&)), this ,SLOT(putIPSlot(const QByteArray&)));
		QObject::disconnect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));

	}
}

/*
 *  UDP
 */
void execut::udpOpen(bool ck){
	if(ck){
		emit Onlain(true);
		//из порта
		QObject::connect(usocket, SIGNAL(readyRead()), this, SLOT(readUDPSlot()));
		//посылка команд из данного класса в порт
		QObject::connect(this, SIGNAL(putCh(const QByteArray&)), this ,SLOT(putUDPSlot(const QByteArray&)));
	}else{
        emit Onlain(false);
		//из порта
		QObject::disconnect(usocket, SIGNAL(readyRead()), this, SLOT(readUDPSlot()));
		//посылка команд из данного класса в порт
		QObject::disconnect(this, SIGNAL(putCh(const QByteArray&)), this ,SLOT(putUDPSlot(const QByteArray&)));
	}
}

/*
 * EXT
 */
void execut::extOpen(bool ck){
	if(ck){
		emit Onlain(true);
		QObject::connect(this, SIGNAL(resExt(const QByteArray&)), this, SLOT(Receive(const QByteArray&)));
	  //посылка команд из данного класса
		QObject::connect(this, SIGNAL(putCh(const QByteArray&)), this ,SLOT(putEXT(const QByteArray&)));
	}else{
		emit Onlain(false);
		QObject::disconnect(this, SIGNAL(resExt(const QByteArray&)), this, SLOT(Receive(const QByteArray&)));
		//посылка команд из данного класса
		QObject::disconnect(this, SIGNAL(putCh(const QByteArray&)), this ,SLOT(putEXT(const QByteArray&)));
	}
}

void execut::error(QAbstractSocket::SocketError err){
	QTcpSocket*  sock=qobject_cast<QTcpSocket*>(QObject::sender());
	emit messageError("Critical TCP error "+sock->peerAddress().toString()+":"+QString::number(sock->peerPort ())
	+" err:"+QString::number((int)err)+sock->errorString());
	Stop();
}



/**
 * Слот - читаем из порта
 */
void execut::readIPSlot(){

	int    fsz,osz;
	ucrc   id;
	resp.append(socket->readAll());

	 while (resp.size()>6) {
		 fsz=(unsigned char)resp[5];
		 if(resp.size()-6 >= fsz){
			 osz=resp.size()-(fsz+6);
			 id.cha[0]=resp[1];
			 id.cha[1]=resp[0];
			 gresp.resize(fsz);
			 memcpy(gresp.data(),resp.data()+6,fsz);
			 if(osz>0) memmove(resp.data(),resp.data()+6+fsz,osz);
			 resp.resize(osz);
			 if(idtran.ushor==id.ushor){//проверка id транзакции
				//   if(debug) qDebug()<<"TCP sender:"+sender.toString();
				// ((concommand*)curCMD)->AddrRF=socket->peerAddress();
				  Receive(gresp);
			  }else{
				   emit messageError(QString::number((unsigned char)gresp[0])
				   +":Delay longer waiting time");
			   }
	     }
	 }
}

/**
 * Слот - посылаем массив в IP порт
 */
void execut::putIPSlot(const QByteArray& arr){

	if(socket->write(arr)<0){
		if(debug){
			qDebug()<<"Error write socket"<<socket->error()<<socket->errorString();
		}

	}else socket->flush();
}


/**
 * Слот - читаем из порта
 */
void execut::readUDPSlot(){

qint64 sz;
int    fsz,osz;
ucrc   id;

 while (usocket->hasPendingDatagrams()) {
	   sz=usocket->pendingDatagramSize();
	   osz=resp.size();
	   resp.resize(osz+sz);
	   usocket->readDatagram(resp.data()+osz,sz,&sender, &senderPort);
//	  qDebug()<<"sender:"<<sender.toString()<<senderPort<<Port;
	   if((udpAddrRes==sender || udpAddrRes==QHostAddress(QHostAddress::Broadcast)) && senderPort==Port){
		   while (resp.size()>6) {
			   fsz=(unsigned char)resp[5];
			   if(fsz>=resp.size()-6){
				   id.cha[0]=resp[1];
				   id.cha[1]=resp[0];
				   osz=resp.size()-(fsz+6);
				   gresp.resize(fsz);
				   memcpy(gresp.data(),resp.data()+6,fsz);
				   if(osz>0) memmove(resp.data(),resp.data()+6+fsz,osz);
				   resp.resize(osz);
				   if((idtran.ushor==id.ushor || udpAddrRes==QHostAddress(QHostAddress::Broadcast)) && curCMD!=NULL){//проверка id транзакции
					  if(debug) qDebug()<<"UDP from:"+sender.toString();
					  udpAddrRF=sender;
					  ((concommand*)curCMD)->AddrRF=sender;
					  Receive(gresp);
				   }else{
					   emit messageError(QString::number((unsigned char)gresp[0])
					   +":Delay longer waiting time");
				   }
			   }
		   }
	   }else{
		   emit messageError("SRC Udp is bad!");
		   gresp.clear();
	   }
   }
}

/**
 * Слот - посылаем массив в IP порт
 */
void execut::putUDPSlot(const QByteArray& arr){
	qint64 wr;
	if(usocket) wr=usocket->writeDatagram(arr, udpAddrSen, Port);
	//qDebug()<<"!!"<<udpAddrSen.toString()<<wr;
}


void execut::putEXT(const QByteArray& sen){

	extCOM->wait=curWait;//mMap::TimeOut;?
	extCOM->setRez(NOT);
	extCOM->setERR(NOERR);
	extCOM->setAFCmd(sen);

	if(sen.at(0)==0x00) extCOM->Type=BROADCAST;
	else        		extCOM->Type=IF;
	if(extExe!=NULL){
		if(extExe->getTransport()!=NONE){
			if(debug){
				qDebug()<<"EXT send from"<<Tape->marker<<":"<<sen.size();
				//qDebug()<<"->"+ByteArray_to_String(sen);
			}
			extExe->exeCommandE(extCOM);
			return;  //отправили

		}
	}

	if(debug) qDebug()<<"EXT conn bad"<<Tape->marker;
	//не отправили
	extCOM->setRez(BAD);
	emit resExt(emptyArr);

}
/*
 * Неблокируем
 */
void execut::exeCommandE(dIF* CMD){

	 if(execut* exe=qobject_cast<execut*>(CMD->parentEXE)) CMD->Retrans=NoBlockRet;
	 else                                                  CMD->Retrans=ExternRet;
	 if(transp!=NONE) {
		 mutex.lock();
		 queueCMD.enqueue(CMD);
	     mutex.unlock();
	     emit pendTime(false);
	 }else{
    	 CMD->setRez(ERRCON);
    	 extReceive((command*)CMD,emptyArr);
     }
}


//*********
QString execut::getAddress(){
	QString rez;
	switch(transp){
		case NONE:
			rez="";
			break;
		case TCP:
		case UDP:
			rez=Addr.toString();
			break;
		case COM:
			rez=comPort;
			break;
		case EXT:
			if(extExe!=NULL) rez=extExe->getAddress();
			break;
	}
	return rez;
}

QString execut::getPort(){
	QString rez;
	switch(transp){
		case NONE:
		case COM:
		case EXT:
			rez="";
			break;
		case TCP:
		case UDP:
			rez=rez.setNum(Port,10);
			break;
	}
	return rez;
}




void execut::setTape(tape* T){
	if(run) Stop();
   	if(Tape!=&TapeI){
   		if(mdebug) qDebug()<<"del old tap";
   		delete(Tape);
   	}else             Tape->clear();
   	if(T) Tape=T;
   	else  Tape=&TapeI;
}

// СЛУЖЕБНЫЕ ФУНКЦИИ


QByteArray execut::convert(const QString& str){
	bool ok;
	QByteArray send(str.size()/2,0);
	unsigned short rez;
	int i;

	for( i=0;i<str.size()/2;i++){
		rez=str.mid(i*2,2).toUShort(&ok,16);
		if(ok) send[i]=(unsigned char)rez;
		else break;
	}
	send.resize(i);
	return send;
}

/*
 * преобразует обьединение в битовый массив заданной длинны
 */
QByteArray execut::convert(lon u,int l){
	QByteArray rez;
	if(l>8) l=8;
	for(int i=l;i>0;i--) rez.append(u.cha[i-1]);
	return rez;
}
/*
 * преобразует число в битовый массив
 */
QByteArray execut::convert(unsigned long long int ch,int l){
	lon u;
	u.din=(unsigned long long int)ch;
	return convert(u,l);
}
/*
 *  битовый массив в число
 *  Аргументы массив, первый символ, длинна
 */
long long int execut::aconvert(QByteArray ba,int n,int l){
	lon u;
	if(l>8) l=8;
	u.din=0; //забили нулями
	if(ba.count()>=n+l) {//проверка на выход за пределы
		for(int i=0;i<l;i++) u.cha[i]=ba.at(n+l-i-1);
	}
	return (long long int)u.din;
}


//*************************
QString execut::ByteArray_to_String(const QByteArray& arr){

	QString st,fst;
	for(int i=0;i<arr.size();i++){
			//разбор байтов
			st.setNum(arr.at(i),16);
			if(st.size()<2) st="0"+st;
			st=st.right(2);
			fst.append(st);
	}
	return fst;
}

QString execut::Char_to_FString16(const char ch){

	QString st;
	//разбор байтов
	st.setNum(ch,16);
	if(st.size()<2) st="0"+st;
	st=st.right(2);
	return st;
}

QString execut::Char_to_FString10(const char ch){

	QString st;
	//разбор байтов
	st.setNum(ch,10);
	if(st.size()<2) st="0"+st;
	st=st.right(2);
	return st;
}



