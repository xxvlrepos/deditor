/*
 * mdbServer.cpp
 *
 *  Created on: 23 июня 2014 г.
 *      Author: Usach
 */

#include "mdbServer.h"


#ifndef _WIN_
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
#else

 //  #include <winsock2.h> // необходимые функции под виндой
/*
    #define SIO_KEEPALIVE_VALS _WSAIOW(IOC_VENDOR,1)

	struct tcp_keepalive {
		u_long onoff;
		u_long keepalivetime;
		u_long keepaliveinterval;
	};
 */

#endif



SSoc::SSoc(QTcpSocket* S,mThread* thread):QObject(0){
	   Socket=S;
	   parentTr=thread;
	   connect(Socket,SIGNAL(readyRead()),this,SLOT(onReadyRead()));
	   connect(Socket,SIGNAL(disconnected()),parentTr,SLOT(onDisconnected()));

	   Socket->moveToThread(parentTr);
	   error=false;
	   block=false;
	   closeS=false;
	   tran.reserve(260);
	   res.reserve(260);
	   CMD.parentEXE=(QObject*)parentTr;
	   port=Socket->peerPort();
	   adr=Socket->peerAddress().toString();
	   moveToThread(parentTr);
 }
SSoc::~SSoc(){
	 disconnect(Socket);
	 if(block) qWarning()<<"first close dataServer!!!";
	 if(Socket->isOpen()) Socket->close();
	 Socket->deleteLater();
}

int SSoc::read(){
	qint64 sz;
	int    fsz,osz=tran.size();
	while (!Socket->atEnd()) {
		   sz=Socket->bytesAvailable();
		   if(sz+osz>260) sz=260-osz;
		   tran.resize(osz+sz);
		   Socket->read(tran.data()+osz,sz);
		   if(tran.size()<=6) continue;
		   fsz=(unsigned char)tran[5];

		   if(fsz>=tran.size()-6){
			   if(tran[2] || tran[3] || tran[4]) return -1;//проверка

			   if(!block){//буфер свободен
				   block=true;
				   res=tran.left(8);
				   CMD.setAFCmd(tran.mid(6,fsz));
				   CMD.wait=mMap::TimeOut;
				   CMD.data.clear();
			   }else{//занято
				   QByteArray r(tran.left(8));
				   r.resize(9);
				   r[5]=3;
				   r[7]=r.at(7)+0x80;
				   r[8]=0x06;
				   write(r);
				   error=true;
			   }
			   osz=tran.size()-(fsz+6);
			   memmove(tran.data(),tran.data()+6+fsz,osz);
			   tran.resize(osz);
			   return 1;
		   }else osz=tran.size();
	}
	return 0;
}


void SSoc::write(){
	if(!error) write(res);
	error=false;
	block=false;
}

void SSoc::write(const QByteArray& r){

	if(Socket->isOpen()){
		Socket->write(r);
		Socket->flush();
	}
}


void SSoc::receive(){
	if(closeS){
		block=false;
		disconnect(Socket,SIGNAL(disconnected()),parentTr,SLOT(onDisconnected()));
		Socket->close();
		QMetaObject::invokeMethod(parentTr,"remove",Qt::QueuedConnection,Q_ARG(SSoc*, this));
		return;
	}

	if(CMD.data.size()>0 && CMD.getRez()==OK){
		res[5]=(unsigned char)CMD.data.size();
		res.replace(6,2,CMD.data);
	}else{
		res.resize(9);
		res[5]=3;
		res[7]=res.at(7)+0x80;
		res[8]=0x04; //доступно, но ответ не получен - ошибка
	}
	write();
}

void SSoc::onReadyRead(){

	switch(read()){
		case -1:
			qCritical()<<"Error data. Disconnect.";
			Socket->close();
			return;
		case 0:
			return;
		case 1:
			parentTr->appendComand(this);
			break;
	}
}


//*********************************************************************************
mThread::mThread(mMap* m, QObject *parent) : QThread(parent){
	map=m;
	qRegisterMetaType<dIF*>("dIF*");
	qRegisterMetaType<QTcpSocket*>("QTcpSocket*");
	qRegisterMetaType<SSoc*>("SSoc*");
}

void mThread::appendSocket(QTcpSocket* soc){
	 SocketList.append(new SSoc(soc,this));
}


void mThread::run(){
	exec();
}

/*
void mThread::append(QTcpSocket* s){
	 QMetaObject::invokeMethod(this,"appendSocket",Qt::QueuedConnection,Q_ARG(QTcpSocket*, s));
}
*/

/*
void mThread::stop(QTcpSocket* ns){
	if(SocketList.contains(ns)){
		disconnect(SocketList[SocketList.search(ns)]->Socket,SIGNAL(disconnected()), this, SLOT(onDisconnected()));
		SocketList.remove(ns);
	}
}
*/

void mThread::stop(){
	int i=SocketList.size();
	SocketList.closeAll();
	qWarning()<<"close all MODBUS connection ("+ QString::number(i)+")";
	quit();
}




void mThread::onDisconnected(){
	QTcpSocket* Socket = qobject_cast<QTcpSocket*>(sender());
	int i=SocketList.search(Socket);
	if(i>=0){
		SSoc* s=SocketList.at(i);
		if(!s->isBlock()) 	remove(s);
		else s->closeS=true;
	}
}

void mThread::remove(SSoc* soc){
	QString txt(soc->adr+":"+QString::number(soc->port)+" disconnect");
	qWarning()<<txt;
	SocketList.remove(soc);
}


void mThread::extReceive(command* dif){
	int i=SocketList.search((dIF*)dif);
	if(i>=0){
		SSoc* s=SocketList.at(i);
		QMetaObject::invokeMethod(s,"receive",Qt::QueuedConnection);
	}
}


void mThread::appendComand(SSoc* s){

	unsigned char adr=s->CMD.adr();
	unsigned char fun=s->CMD.fun();
	if(!map->contains(adr))       {writeBad(s,0x0b); return;}//нет устройства
	if(!map->value(adr)->dstatus) {writeBad(s,0x0b); return;}//отвалилось

	if(fun==0x14)                		   fun14(s);//файловое из буфера
	else if(fun==0x04){
		 int ad;
		 QByteArray com=s->CMD.getCmd();
         ad=(unsigned char)com[0];
		 ad=(ad<<8)+(unsigned char)com[1];
		 if(ad<MArray::maxBAdr)            fun04(s);  //попали в буфер
		 else                              retrans(s);//не попали - ретраслятор
	}else                                  retrans(s);//остальные - ретранслятор
}

void mThread::retrans(SSoc* s){
   unsigned char adr=s->CMD.adr();
   MArray* ma;
   ma=map->value(adr);
   if(ma->exe){
	  if(ma->exe->getTransport() && ma->type==MArray::kNONE){//есть связь и тип с автоопределением, т.е. модбас
		  QMetaObject::invokeMethod(ma->exe,"exeCommandE",Qt::QueuedConnection,Q_ARG(dIF*, &(s->CMD)));
	  }
	  else writeBad(s,0x04);
   }
   else  writeBad(s,0x04);
}

void mThread::writeBad(SSoc* s,unsigned char bad){
   		s->res.resize(9);
   		s->res[5]=3;
   		s->res[7]=s->res.at(7)+0x80;
   		s->res[8]=bad;
   	    s->write();
}

void mThread::fun04(SSoc* s){
	   int ad,le;
	   QByteArray com(s->CMD.getAFCmd());
	   if(com.size()!=6)  {writeBad(s,0x03); return;}
	   le=(unsigned char)com[5];
	   ad=(unsigned char)com[2];
	   ad=(ad<<8)+(unsigned char)com[3];

	   if(le<1 || le>0x7D) {writeBad(s,0x03); return;}
	   if(ad<MArray::Shift || ad+le>=MArray::maxBAdr) { writeBad(s,0x02); return;}
	   s->res.append(le*2);
	   s->res.append(map->value(s->CMD.adr())->midM(2*(ad-MArray::Shift),2*le));
	   s->res[5]=le*2+3;
	   s->write();
}


void mThread::fun14(SSoc* s){
	   int fl,ad,le;
	   int rl=0;
	   QByteArray com(s->CMD.getAFCmd());
	   fl=(unsigned char)com[2];
	   if(com.size()-3!=fl)  {writeBad(s,0x03); return;}
	   s->res.append(0x01);//просто резервируем место в поз 8
	   for(int i=0;i<fl/7;i++){
		   le=(unsigned char)com[9+7*i];
		   ad=(unsigned char)com[6+7*i];
		   ad=(ad<<8)+(unsigned char)com[7+7*i];
		   //проверки
		   if(le<1 || le>0x7D) {writeBad(s,0x03); return;}
		   if((uchar)com[3+7*i]!=0x06 || (uchar)com[4+7*i]!=0 || (uchar)com[5+7*i]!=0x01) {writeBad(s,0x03); return;}
		   if(ad>=MArray::maxFAdr) { writeBad(s,0x02); return;}
		   //*****
		   s->res.append(le*2+1);
		   s->res.append(0x06);
		   s->res.append(map->value(s->CMD.adr())->midM(2*ad,2*le));
		   rl+=(2+le*2);
	   }
	   s->res[5]=rl+3;
	   s->res[8]=rl;
	   s->write();
}

//**************************************************************************
mdbServer::mdbServer(mMap* m,QObject *parent) : QTcpServer(parent) {
	// TODO Auto-generated constructor stub
	map=m;
	ThredRec=NULL;
}

mdbServer::~mdbServer() {
	stop();
}

void mdbServer::start(QDomElement sElem){

	if(sElem.isNull())             return;
	if(sElem.nodeName()!="server") return;
	ser=sElem;

	allowedIP.clear();
	bool ok;
	QString port, maxNumConn,ipListen;
	port=ser.attribute("port","502");
	maxNumConn=ser.attribute("maxNumConn","10");
	ipListen=ser.attribute("ipListen","0.0.0.0");

	lport=port.toInt(&ok,10);
	maxNumC=maxNumConn.toInt(&ok,10);
	haddr.setAddress(ipListen);
	setMaxPendingConnections (maxNumC);

	QHostAddress a,b;
	QDomNodeList nl=ser.childNodes();
	for(int i=0;i<nl.size();i++){
		QString sa,sb;
		sa=nl.at(i).toElement().attribute("firstIP","");
		sb=nl.at(i).toElement().attribute("lastIP","");
		if(sa!="" && sb!=""){//диапазон
			a.setAddress(sa);
			b.setAddress(sb);
		}else if(sa=="0.0.0.0"){//один и причем все
			a.setAddress("0.0.0.0");
			b.setAddress("255.255.255.255");
		}else{//один
			a.setAddress(sa);
			b.setAddress(sa);
		}
		allowedIP.append(dia(a.toIPv4Address(),b.toIPv4Address()));
	}

	if (!listen(haddr,lport)) {
		    qCritical()<< tr("Unable to start the server: %1.").arg(errorString());
	}else{
	    	qWarning()<<"start MODBUS server "+haddr.toString()+":"+port;
	}
	work =true;
}

void mdbServer::stop(){


	if(ThredRec){
		disconnect(ThredRec, SIGNAL(finished()), this, SLOT(deleteThred()));
		ThredRec->stop();
		ThredRec->wait();
		delete(ThredRec);
		ThredRec=NULL;
	}

	if(isListening ()) 	close();
	if(work) qWarning()<<"stop MODBUS server";
	work=false;
}


void mdbServer::incomingConnection(qintptr socketDescriptor)
{

	QString txt;
	QTcpSocket* tcpSocket;
	tcpSocket= new QTcpSocket();
	qDebug()<<"INCOMING";
	if (!tcpSocket->setSocketDescriptor(socketDescriptor)) {
	    emit error(tcpSocket->error());
	    delete(tcpSocket);
	    return;
	}

	QHostAddress addr=tcpSocket->peerAddress();
	quint32 iaddr=addr.toIPv4Address();

	if(ThredRec) if(ThredRec->getSize()>=maxNumC) {
	   qCritical()<<addr.toString()+":"+txt.setNum(tcpSocket->peerPort(),10)+" Exceeded the number of connections!";
	   tcpSocket->close();
	   delete(tcpSocket);
	   return;
   }
	bool allow=false;
	for(int i=0;i<allowedIP.size();i++){
		if(allowedIP[i].first<=iaddr && allowedIP[i].last>=iaddr){
			allow=true;
			break;
		}
	}
	if(!allow){
		 qCritical()<<addr.toString()+":"+txt.setNum(tcpSocket->peerPort(),10)+" Prohibited client address!";
		 tcpSocket->close();
		 delete(tcpSocket);
		 return;
	}

   qWarning()<<addr.toString()+":" +txt.setNum(tcpSocket->peerPort())+" connect";

#ifndef _WIN_
    int enableKeepAlive = 1;
    int maxIdle = 10;     /* seconds */
    int count = 3;        // send up to 3 keepalive packets out, then disconnect if no response
    int interval = 2;    // send a keepalive packet out every 2 seconds (after the 5 second idle period)
   setsockopt(socketDescriptor, SOL_SOCKET, SO_KEEPALIVE, &enableKeepAlive, sizeof(enableKeepAlive));
   setsockopt(socketDescriptor, IPPROTO_TCP, TCP_KEEPIDLE, &maxIdle, sizeof(maxIdle));
   setsockopt(socketDescriptor, SOL_TCP, TCP_KEEPCNT, &count, sizeof(count));
   setsockopt(socketDescriptor, SOL_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
#else
 /*
    tcp_keepalive keepAliveSettings;
   DWORD  BytesReturned;
   //включить keepalive
   keepAliveSettings.onoff=enableKeepAlive;
   keepAliveSettings.keepalivetime=maxIdle*1000;
   keepAliveSettings.keepaliveinterval=interval*1000;
   WSAIoctl(socketDescriptor, SIO_KEEPALIVE_VALS, &keepAliveSettings, sizeof(keepAliveSettings), NULL, 0, &BytesReturned, NULL, NULL);
*/
//   char enableKeepAlive = 1;
//   setsockopt(socketDescriptor, SOL_SOCKET, SO_KEEPALIVE, &enableKeepAlive, sizeof(enableKeepAlive));
#endif

   if(ThredRec==NULL){
	   ThredRec = new mThread(map);
	   connect(ThredRec, SIGNAL(finished()), this, SLOT(deleteThred()));
	   ThredRec->start();
	   ThredRec->appendSocket(tcpSocket);
   }else{
	   ThredRec->appendSocket(tcpSocket);
   }
}

void mdbServer::deleteThred(){
	ThredRec->wait();
	delete(ThredRec);
	ThredRec=NULL;
}
