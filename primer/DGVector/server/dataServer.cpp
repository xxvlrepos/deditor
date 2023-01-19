/*
 * dataServer.cpp
 *
 *  Created on: 06 июня 2014 г.
 *      Author: Usach
 */

#include "dataServer.h"
#include "../com485/com485.h"

extern  mMap MDBArray;

//*********************************************************************************************
uchar dataServer::NumDebugDev=0;
uchar dataServer::NumDebugCycle=0;

const QVector<unsigned short> dataServer::mdbsADC=IniQVector<unsigned short>()<<0x1000<<0x1100<<0x1200<<0x1300<<0x1700<<0x1800;
const QVector<unsigned short> dataServer::mdbsTAH=IniQVector<unsigned short>()<<0x1400<<0x1410<<0x1420;
const QVector<unsigned short> dataServer::mdbsE=IniQVector<unsigned short>()<<0x1500<<0x1510;
const QVector<unsigned short> dataServer::mdbsF=IniQVector<unsigned short>()<<0x1600<<0x1610;
const QVector<unsigned short> dataServer::mdbsTERM=IniQVector<unsigned short>()<<0x1900<<0x1910<<0x1920;


dataServer::dataServer(mMap* m,QObject *parent): QObject(parent){
	map=m;
	uThread=NULL;
	fullMirror=true;
}

dataServer::~dataServer() {
	stop();
}


bool dataServer::start(QDomElement cli,QDomDocument mdb,bool fullMirr){

	bool ok;
	mUDP.clear();
	mTCP.clear();
	mCOM.clear();
	mdbDoc=mdb;
	fullMirror=fullMirr;
	if(mdb.isNull()) fullMirror=true;

	QString repeat, samplingRate, samplingIdle, timeOut, type;

	repeat=cli.attribute("repeat", "0");
	samplingRate=cli.attribute("samplingRate", "250");
	samplingIdle=cli.attribute("samplingIdle", "5000");
	timeOut=cli.attribute("timeOut", "100");

	Repeat=repeat.toInt(&ok, 10);
	SamplingIdle=samplingIdle.toInt(&ok, 10);
	SamplingRate=samplingRate.toInt(&ok, 10);
	TimeOut=timeOut.toInt(&ok, 10);

	map->setParam(TimeOut);//для ретранслятора

	QDomNodeList cpl;
	QDomNodeList cl=cli.childNodes();
	QDomElement el, ec;
	int cou=0;

	unsigned char imdb;
	MArray* ar;
	QString ip, port, speed, comaddr;

	for(int i=0; i < cl.size(); i++){
		el=cl.at(i).toElement();
		if(el.nodeName() == "UDP"){
			imdb=el.attribute("modbus").toUShort(&ok, 10);

			if(!map->contains(imdb)){
				map->insert(imdb, new MArray());
				ar=map->value(imdb);
				ar->tipCon="UDP";
				ar->id=el.attribute("id",QString::number(qrand(),16)+QString::number(qrand(),16));
				ar->ip=el.attribute("ipaddr", "");
				ar->port=el.attribute("port", "");
				ar->setModbus(el.attribute("modbus", "255"));
				ar->alias=el.attribute("alias", "");
				ar->emulation=el.attribute("emulation", "000|000000|00");
				mUDP[ar->ip].parCon=ar->port;
				mUDP[ar->ip].mlist.append(imdb);
				mUDP[ar->ip].samplingIdle=SamplingIdle;
				mUDP[ar->ip].samplingRate=SamplingRate;
				mUDP[ar->ip].timeOut=TimeOut;
				mUDP[ar->ip].repeat=Repeat;
				mUDP[ar->ip].fullMirror=fullMirror;
				cou++;
			}else qDebug() << el.attribute("modbus") + " doubling addres - is disable";
		}else if(el.nodeName() == "TCP"){
			ip=el.attribute("ipaddr", "");
			port=el.attribute("port", "");
			cpl=el.childNodes();
			mTCP[ip].parCon=port;
			mTCP[ip].samplingIdle=el.attribute("samplingIdle", samplingIdle).toInt(&ok, 10);
			mTCP[ip].samplingRate=el.attribute("samplingRate", samplingRate).toInt(&ok, 10);
			mTCP[ip].timeOut=el.attribute("timeOut", timeOut).toInt(&ok, 10);
			mTCP[ip].repeat=el.attribute("repeat", repeat).toInt(&ok, 10);
			mTCP[ip].fullMirror=(bool)el.attribute("fullMirror", "1").toInt(&ok, 10);
			for(int j=0; j < cpl.size(); j++){
				ec=cpl.at(j).toElement();
				imdb=ec.attribute("modbus").toUShort(&ok, 10);
				if(!map->contains(imdb)){
					map->insert(imdb, new MArray());
					ar=map->value(imdb);
					ar->tipCon="TCP";
					ar->id=ec.attribute("id",QString::number(qrand(),16)+QString::number(qrand(),16));
					ar->ip=ip;
					ar->port=port;
					ar->setModbus(ec.attribute("modbus", "255"));
					ar->alias=ec.attribute("alias", "");
					ar->emulation=ec.attribute("emulation", "000|000000|00");
					mTCP[ip].mlist.append(imdb);
					cou++;
				}else qDebug() << ec.attribute("modbus") + " doubling addres - is disable";
			}
		}else if(el.nodeName() == "COM485"){
			speed=el.attribute("speed", "57600");
			comaddr=el.attribute("comAddr", "");
			cpl=el.childNodes();
			mCOM[comaddr].parCon=speed;
			mCOM[comaddr].proto=el.attribute("protocol", "Modbus");
			mCOM[comaddr].samplingIdle=el.attribute("samplingIdle", samplingIdle).toInt(&ok, 10);
			mCOM[comaddr].samplingRate=el.attribute("samplingRate", samplingRate).toInt(&ok, 10);
			mCOM[comaddr].timeOut=el.attribute("timeOut", timeOut).toInt(&ok, 10);
			mCOM[comaddr].repeat=el.attribute("repeat", repeat).toInt(&ok, 10);
			mCOM[comaddr].fullMirror=(bool)el.attribute("fullMirror", "1").toInt(&ok, 10);
			for(int j=0; j < cpl.size(); j++){
				ec=cpl.at(j).toElement();
				imdb=ec.attribute("modbus").toUShort(&ok, 10);
				type=ec.attribute("type", "A");
				if(!map->contains(imdb)){
					map->insert(imdb, new MArray());
					ar=map->value(imdb);
					ar->tipCon="COM485";
					ar->id=ec.attribute("id",QString::number(qrand(),16)+QString::number(qrand(),16));
					ar->setSpeed(speed);
					ar->comaddr=comaddr;
					ar->setModbus(ec.attribute("modbus", "255"));
					ar->alias=ec.attribute("alias", "");
					ar->emulation=ec.attribute("emulation","000|000000|00");
					if(mCOM[comaddr].proto != "Modbus"){
						if(type == "A"){
							ar->inf.adc[0]=A;
							ar->type=MArray::kA;
						}else if(type == "C"){
							ar->inf.adc[0]=C;
							ar->type=MArray::kC;
						}else if(type == "D"){
							ar->inf.adc[0]=D;
							ar->type=MArray::kD;
						}else if(type == "I"){
							ar->inf.adc[0]=I;
							ar->type=MArray::kI;
						}else if(type == "T"){
							ar->inf.tah[0]=T;
							ar->type=MArray::kT;
						}
						if(ar->type == MArray::kA || ar->type == MArray::kC || ar->type == MArray::kD){
							ar->replaceM(0x00 * 2, (char*) ar->inf.adc.data(), 1);
						}
						if(ar->type == MArray::kT){
							char ch=0x01;
							ar->replaceM(0x0401 * 2, &ch, 1);
						}
					}
					mCOM[comaddr].mlist.append(imdb);
					cou++;
				}else qDebug() << ec.attribute("modbus") + " doubling addres - is disable";
			}
		}
	}

	if(cou == 0) return false;
	if(NumDebugDev > 0)   if(map->contains(NumDebugDev))   map->value(NumDebugDev)->Debug=true;
	if(NumDebugCycle > 0){
			if(map->contains(NumDebugCycle)){
				ar=map->value(NumDebugCycle);
				ar->Tdebug=true;//сам
				//и для всех на одном с ним соединении
				if(ar->tipCon=="COM485"){
					for(int i=0; i<mCOM[ar->comaddr].mlist.size();i++){
						map->value(mCOM[ar->comaddr].mlist[i])->Tdebug=true;
					}
				}
				if(ar->tipCon=="TCP"){
					for(int i=0; i<mTCP[ar->ip].mlist.size();i++){
						map->value(mTCP[ar->ip].mlist[i])->Tdebug=true;
					}
				}
			}
	}

	ExeListThread *thread;

	conMap::iterator itu=mUDP.begin();
	QList<tape*> listT;
	for(; itu != mUDP.end(); itu++){
		listT.append(addUDP_LT(itu.key(), itu.value().parCon, itu.value().mlist.at(0),itu.value()));
	}
	if(listT.size() > 0){

		thread=new ExeListThread(listT, map);

		connect(thread, SIGNAL(finished(int)), this, SLOT(exitRun(int)));
		connect(thread, SIGNAL(finStat(int)), this, SIGNAL(finStat(int)));
		connect(thread, SIGNAL(finDin(int)), this, SIGNAL(finDin(int)));
		connect(thread, SIGNAL(message(QString,int)), this, SLOT(sendMess(QString,int)));

		ThreadList.append(thread);
		thread->moveToThread(thread);
	}

	conMap::iterator itt=mTCP.begin();
	for(; itt != mTCP.end(); itt++){

		thread=new ExeListThread(addTCP_LT(itt.key(), itt.value()), map);

		connect(thread, SIGNAL(finished(int)), this, SLOT(exitRun(int)));
		connect(thread, SIGNAL(finStat(int)), this, SIGNAL(finStat(int)));
		connect(thread, SIGNAL(finDin(int)), this, SIGNAL(finDin(int)));
		connect(thread, SIGNAL(message(QString,int)), this, SLOT(sendMess(QString,int)));

		ThreadList.append(thread);
		thread->moveToThread(thread);

	}

	conMap::iterator itc=mCOM.begin();
	for(; itc != mCOM.end(); itc++){

		if(itc.value().proto == "VectorL"){
			snifferThread* sniffer;
			QVector<unsigned char> ml=itc.value().mlist.toVector();
			sniffer=new snifferThread(itc.key(), itc.value().parCon, ml,itc.value().timeOut,itc.value().samplingIdle, map);
			snifferTList.append(sniffer);
			connect(sniffer, SIGNAL(finStat(int)), this, SIGNAL(finStat(int)));
			connect(sniffer, SIGNAL(finDin(int)), this, SIGNAL(finDin(int)));
			connect(sniffer, SIGNAL(message(QString,int)), this, SLOT(sendMess(QString,int)));

			sniffer->start();
			sniffer->moveToThread(sniffer);

		}else{
			ExeListThread* thread;
			if(itc.value().proto == "VectorA"){
				thread=new ExeListThread(addCOM_LTV(itc.key(), itc.value()), map);
			}else{
				thread=new ExeListThread(addCOM_LT(itc.key(), itc.value()), map);
			}

			connect(thread, SIGNAL(finished(int)), this, SLOT(exitRun(int)));
			connect(thread, SIGNAL(finStat(int)), this, SIGNAL(finStat(int)));
			connect(thread, SIGNAL(finDin(int)), this, SIGNAL(finDin(int)));
			connect(thread, SIGNAL(message(QString,int)), this, SLOT(sendMess(QString,int)));


			ThreadList.append((QThread*)thread);
			thread->moveToThread(thread);
		}
		qDebug() << itc.key() + " run protocol " + itc.value().proto;


	}

	if(snifferTList.size()>0){
		qWarning() << "start LISTEN client";
	}


	if(ThreadList.size()> 0){
		int sz=ThreadList.size();
		uThread=new UpdateThread(&ThreadList);
		uThread->moveToThread(uThread);
		uThread->start();
		qWarning() << "start DATA client";
	}

	return true;
}

void dataServer::sendMess(QString s,int i) {
	switch(i){
	     case 1: qWarning() << s; break;
	     default:
	    	 qDebug() << s;
	}
}


void dataServer::getMINFO(MArray* ar){
	usort z;
	for(int i=0; i<mdbsADC.size();i++){
		z.sd=ar->getMData(mdbsADC[i]); ar->inf.adc[i]=z.sc[1]; ar->inf.ath[i][0]=z.sc[0];
		z.sd=ar->getMData(mdbsADC[i]+0x0054);
		ar->inf.ath[i][0]=z.sd & 0x000F;
		ar->inf.ath[i][1]=(z.sd>>4) & 0x000F;
		ar->inf.ath[i][2]=(z.sd>>8) & 0x000F;
	}
	for(int i=0; i<mdbsTAH.size();i++){
		z.sd=ar->getMData(mdbsTAH[i]+0x0001);
		if(z.sc[1]!=0) ar->inf.tah[i]=T;
	}
	for(int i=0; i<mdbsE.size();i++){
		z.sd=ar->getMData(mdbsE[i]); if(z.sc[1]!=0) ar->inf.vir[i]=E;
		z.sd=ar->getMData(mdbsF[i]); if(z.sc[1]!=0) ar->inf.vir[i]=F;
	}
	for(int i=0; i<mdbsTERM.size();i++){
		z.sd=ar->getMData(mdbsTERM[i]);
		if(z.sc[1]!=0) ar->inf.term[i]=N;
	}
}



QList<tape*> dataServer::addUDP_LT(QString ip, QString port, unsigned char m,datP p){
	QList<tape*> lt;
	if(m==0) return lt;
	lt.append(exe_TIME(cUDP,ip,port,p.samplingIdle,p.timeOut,m));
	lt.append(exe_I(m,cEXT,ip,port,p));

//	qDebug()<<QString::number(m)+":UDP-run:"+ip;
	return lt;
}

QList<tape*> dataServer::addTCP_LT(QString ip,datP p){
//	QString port,QList<unsigned char>& lmdb,uint timeout

	QList<tape*> lt;
	if(p.mlist.size()==0) return lt;
	unsigned char modbus;
	lt.append(exe_TIME(cTCP,ip,p.parCon,p.samplingIdle,p.timeOut));
	lt.first()->setMaxTcpERRCoun(p.repeat*p.mlist.size());
	for(int i=0;i<p.mlist.size();i++){
		modbus=p.mlist[i];
		qDebug()<<QString::number(modbus)+":TCP-run:"+ip;
		lt.append(exe_I(modbus,cEXT,ip,p.parCon,p));
	}
	return lt;
}

QList<tape*> dataServer::addCOM_LT(QString addr,datP p){
	//QString speed,QList<unsigned char>& lmdb,uint timeout
	QList<tape*> lt;

	if(p.mlist.size()==0) return lt;
	unsigned char modbus;
	lt.append(exe_TIME(cCOM,addr,p.parCon,p.samplingIdle,p.timeOut));
	for(int i=0;i<p.mlist.size();i++){
		modbus=p.mlist[i];
		qDebug()<<QString::number(modbus)+":COM-run:"+addr;
        lt.append(exe_I(modbus,cEXT,addr,p.parCon,p));
	}
	return lt;
}




//генераторы листов опроса для типов соединений

//Установка соединения и поправляем часы раз в час
tape* dataServer::exe_TIME(connectType cType,QString addr,QString par,uint samplingIdle,uint TimeOut,unsigned char m){
	tape* tp=new tape();
	tp->marker=0;
	tp->setMaxRepeat(0);

	//-------------
	dCONNECT *con=(dCONNECT*)tp->append(CONNECT,"CONN");
	con->cType=cType;
	con->cAddr=addr;
	con->cParam=par;
	con->after_delay_BAD=samplingIdle*2;
	con->after_delay_OK=0;
	con->goBAD_L="CONN";
	if(cType==cEXT) con->delay=5000;
	//---------------------------------------------
	dSETTIME *st =(dSETTIME*)tp->append(SETTIME,"TIME");
	st->setAdr(m);
	st->after_delay_BAD=samplingIdle*2;
	st->delay=1000*60*60;//раз в час
	st->goBAD_L="CONN";
	st->goOK_L ="TIME";
	if(m==0){
		st->wait=0;
		st->Type=BROADCAST;
	}else 	st->wait=TimeOut;
	return tp;
}


//Получаем информацию о блоке
tape* dataServer::exe_I(unsigned char m,connectType cType,QString addr,QString par,datP p,bool setTime){
	tape* tp=new tape();
	tp->marker=m;
	tp->setMaxRepeat(0);

	dIF* it;
	//-------------
	dCONNECT *con=(dCONNECT*)tp->append(CONNECT,"CONN");
	con->cType=cType;
	con->cAddr=addr;
	con->cParam=par;
	con->after_delay_BAD=p.samplingIdle;
	con->goBAD_L="CONN";
	con->delay=200;//+((float)qrand()/RAND_MAX)*300;

	//---------------------------------------------
	if(setTime){
		dSETTIME *st =(dSETTIME*)tp->append(SETTIME);
		st->setAdr(m);
		st->after_delay_BAD=p.samplingIdle;
		st->goBAD_L="CONN";
		st->wait=p.timeOut;
	}
	//------------------------------------------------------
	it =(dIF*)tp->append(IF);
	it->setAFCmd(m,0x2B,execut::convert("0E0402"));
	it->addTest(it->getAF(),0);
	it->ControlRez=true;
	it->wait=p.timeOut;
	it->after_delay_BAD=p.samplingIdle;
	it->goBAD_L="CONN";

	//------------------------------------------
    //получаем всю конфигурацию
	QList<QByteArray> list(fileReadArr(genInfReg()));
	for(int i=0;i<list.size();i++)
	{
		it =(dIF*)tp->append(IF);
		it->setAFCmd(m,0x14,list[i]);
		it->addTest(it->getAF(),0);
		it->ControlRez=true;
		it->wait=p.timeOut;
		it->after_delay_BAD=p.samplingIdle;
		it->goBAD_L="CONN";
	}
	//--------------------------------------------
	tp->append(DISCONNECT);
	return tp;
}

//получаем статические и динамические регистры в зависимости от конфига
//регистры info - повторно не читаем
tape* dataServer::exe_SD(unsigned char m,connectType cType,QString addr,QString par,datP p){


	tape* tp=new tape();
	tp->setMaxRepeat(p.repeat);

	tp->marker=m;
	MArray* ar=map->value(m);
	dIF* it;
	QString txt;
	//-------------
	dCONNECT *con=(dCONNECT*)tp->append(CONNECT);
	con->cType=cType;
	con->cAddr=addr;
	con->cParam=par;
	con->goBAD=-1;
	//--------------------------------------------
	QList<QByteArray> listS(fileReadArr(genStatReg(&ar->inf)));
	for(int i=0;i<listS.size();i++)
	{
		it =(dIF*)tp->append(IF);
		it->setAFCmd(m,0x14,listS[i]);
		it->addTest(it->getAF(),0);
		it->ControlRez=true;
		it->wait=p.timeOut;

		it->goBAD=-1;
		if(i==listS.size()-1){//последняя статик
			it->stOK="s"+txt.setNum(m);
		}
	}
	//--------------------------------------------
	QList<QByteArray> listD;

	if(p.fullMirror) listD=fileReadArr(genDinReg(&ar->inf));
	else 			 listD=fileReadArr(genDinReg(&ar->inf,mdbDoc));

	for(int i=0;i<listD.size();i++)
	{
		it =(dIF*)tp->append(IF,"N"+txt.setNum(i,10));
		it->setAFCmd(m,0x14,listD[i]);
		it->addTest(it->getAF(),0);
		it->ControlRez=true;
		it->wait=p.timeOut;

		it->goBAD=-1;
		if(i==0) it->tCommand=START;//начинаем сеч время
		if(i==listD.size()-1){//последняя - даем задержку и зацикливаем
			it->after_delay_OK=p.samplingRate;
			it->goOK_L="N0";
			it->tCommand=(timeCommand)(it->tCommand | STOP); //выдавать время выполнения цикла
			it->stOK="d"+txt.setNum(m);
		}
	}
	tp->append(DISCONNECT);
	return tp;
}

//*************************************************************************************
//для протокола вектор
QList<tape*> dataServer::addCOM_LTV(QString addr,datP p){

	QList<tape*> lt;
	if(p.mlist.size()==0) return lt;
	unsigned char modbus;
	lt.append(exe_OPEN_V(cCOM,addr,p.parCon,p.samplingIdle));

	for(int i=0;i<p.mlist.size();i++){
		modbus=p.mlist[i];
		qDebug()<<QString::number(modbus)+":COM-run:"+addr;
        lt.append(exe_SV(modbus,cEXT,addr,p.parCon,p));
	}
	return lt;
}


//Установка соединения
tape* dataServer::exe_OPEN_V(connectType cType,QString addr,QString par,uint samplingIdle){
	tape* tp=new tape();
	tp->marker=0;
	tp->setMaxRepeat(0);

	//-------------
	dCONNECT *con=(dCONNECT*)tp->append(CONNECT,"CONN");
	con->cType=cType;
	con->cAddr=addr;
	con->cParam=par;
	con->cProt485=Vector;
	con->after_delay_BAD=samplingIdle*2;
	con->after_delay_OK=0;
	con->goBAD_L="CONN";
	con->after_delay_BAD=5000;

	//---------------------------------------------
	//пустышка
	dNOOP *np =(dNOOP*)tp->append(NOOP,"NOOP");
	np->after_delay_OK=10000;// *60;//раз в минуту
	np->goOK_L ="NOOP";
	//--------------------------------

	tp->append(DISCONNECT);
	return tp;
}

//Опрос статики
tape* dataServer::exe_SV(unsigned char m,connectType cType,QString addr,QString par,datP p){


	tape* tp=new tape();
	tp->setMaxRepeat(0);

	tp->marker=m;
	MArray* ar=map->value(m);
	dIF* it;
	QString txt;
	//-------------
	dCONNECT *con=(dCONNECT*)tp->append(CONNECT,"CONN");
	con->cType=cType;
	con->cAddr=addr;
	con->cParam=par;
	con->cProt485=Vector;
	con->after_delay_BAD=p.samplingIdle;
	con->goBAD_L="CONN";
	con->delay=200;//+((float)qrand()/RAND_MAX)*300;

	//--------------------------------------------
	it =(dIF*)tp->append(IF);
	usort kom;
	int sz=9;
	kom.sd=(sz+1)<<5;
	QByteArray cmd;
	cmd.append(kom.sc[1]);
	cmd.append(kom.sc[0]);
	cmd.append((char)0x00);
	cmd.append((char)0x00);
	switch(ar->type){
		case MArray::kA: it->setAFCmd(m,0x09,cmd); break;
		case MArray::kT: it->setAFCmd(m,0x09,cmd); break;
		case MArray::kC: it->setAFCmd(m,0x09,cmd); break;
		case MArray::kD: it->setAFCmd(m,0x09,cmd); break;
		case MArray::kI: it->setAFCmd(m,0x09,cmd); break;
	}
	it->msize=sz*2;
	it->ControlRez=true;
	it->wait=p.timeOut;
	it->goBAD_L="CONN";
	it->after_delay_BAD=p.samplingIdle;
	it->stOK="s"+txt.setNum(m);

	//--------------------------------------------------------
	tp->append(DISCONNECT,"DISCONN");
	return tp;
}

//Опрос динамики
tape* dataServer::exe_DV(unsigned char m,connectType cType,QString addr,QString par,datP p){


	tape* tp=new tape();
	tp->setMaxRepeat(p.repeat);//нет повторов

	tp->marker=m;
	MArray* ar=map->value(m);
	dIF* it;
	QString txt;
	//-------------
	dCONNECT *con=(dCONNECT*)tp->append(CONNECT,"CONN");
	con->cType=cType;
	con->cAddr=addr;
	con->cParam=par;
	con->cProt485=Vector;
	con->goBAD=-1;

	//--------------------------------------------
	it =(dIF*)tp->append(IF,"DIN");

	unsigned char fun;
	short sz;

	switch(ar->type){
		case MArray::kA: fun=0x07; sz=17; break;
		case MArray::kT: fun=0x02; sz=2; break;
		case MArray::kC: fun=0x04; sz=9; break;
		case MArray::kD: fun=0x02; sz=2; break;
		case MArray::kI: fun=0x02; sz=4; break;
	}
	usort kom;
	kom.sd=(sz+1)<<5;
	QByteArray cmd;
	cmd.append(kom.sc[1]);
	cmd.append(kom.sc[0]);
	cmd.append((char)0x00);
	cmd.append((char)0x00);
	it->setAFCmd(m,fun,cmd);
	it->msize=sz*2;
	it->wait=p.timeOut;
	it->after_delay_OK=p.samplingRate;
	it->goOK_L="DIN";
	it->ControlRez=true;
	it->stOK="d"+txt.setNum(m);

	//--------------------------------------------------------
	tp->append(DISCONNECT);
	return tp;
}


//************************************************************************************************

QList<ushort> dataServer::stringRegToUShort(const QStringList& ls){
    QList<ushort> SR;//список регистров
    int l,a;
    bool ok;
	for(int i=0;i<ls.size();i++){
		l=ls[i].right(2).toInt(&ok,16);
		a=ls[i].left(4).toInt(&ok,16)+0x1000;
		for(int j=0;j<l;j++){
			SR.append(a);
			a++;
		}
	}
	return SR;
}
/*
 * Списки конфигурационных регистров
 */
QStringList dataServer::genInfReg(){
	QStringList getL;
	for(int i=0;i<mdbsADC.size();i++){
		getL<<genSTRL(mdbsADC[i]-0x1000,0x0000,1);//типы каналов и 1 тах
		getL<<genSTRL(mdbsADC[i]-0x1000,0x0054,1);//привязки тахометров
	}
	for(int i=0; i<mdbsTAH.size();i++) getL<<genSTRL(mdbsTAH[i]-0x1000,0x0001,1);//тахометры
	for(int i=0; i<mdbsE.size();i++)   getL<<genSTRL(mdbsE[i]-0x1000,0x0000,1);//канал E
	for(int i=0; i<mdbsF.size();i++)   getL<<genSTRL(mdbsF[i]-0x1000,0x0000,1);//канал F
	for(int i=0; i<mdbsTERM.size();i++) getL<<genSTRL(mdbsTERM[i]-0x1000,0x0000,1);//канал TERM
	return getL;
}


/*
 * Списки статических регистров
 */
QStringList dataServer::genStatReg(ushort ba, uchar t){
	QStringList getL;
	if(ba<0x1000){
		qDebug()<<"Error! Bad base addr!";
		return getL;
	}

	switch(t){
		case A://A
			getL<<genSTRD(ba-0x1000,0x0009,0x0018); //0x1000
			getL<<genSTRL(ba-0x1000,0x0055,1);
			getL<<genSTRL(ba-0x1000,0x0057,1);
			getL<<genSTRL(ba-0x1000,0x0059,1);
			getL<<genSTRD(ba-0x1000,0x007A,0x0081);
			getL<<genSTRD(ba-0x1000,0x0083,0x008A);
			getL<<genSTRD(ba-0x1000,0x0091,0x00A8);
			getL<<genSTRD(ba-0x1000,0x00AA,0x00B1);
			getL<<genSTRD(ba,0x0000,0x004F);//0x2000
			break;
		case C://C
			getL<<genSTRD(ba-0x1000,0x0026,0x0045);//0x1000
			getL<<genSTRL(ba-0x1000,0x0055,1);
			getL<<genSTRD(ba,0x0000,0x0027);//0x2000
			break;
		case D://D
			getL<<genSTRD(ba-0x1000,0x003E,0x0045);//0x1000
			getL<<genSTRD(ba,0x0000,0x0009);//0x2000
			break;
		case M://M повторно-чтобы был запрос
			getL<<genSTRL(ba-0x1000,0x0000,1);//0x1000
			break;
		case I://I
			getL<<genSTRD(ba-0x1000,0x0026,0x002D);//0x1000
			getL<<genSTRD(ba-0x1000,0x0036,0x003D);
			getL<<genSTRD(ba,0x0000,0x0013);//0x2000
			break;
		case T://T
			getL<<genSTRL(ba-0x1000,0x0000,1);//0x1000
			getL<<genSTRD(ba-0x1000,0x0003,0x000A);
			getL<<genSTRD(ba,0x0000,0x0009);//0x2000
			break;
		case E://E
			getL<<genSTRD(ba-0x1000,0x0002,0x0005);//0x1000
			getL<<genSTRD(ba,0x0000,0x0009);//0x2000
			break;
		case F:
			getL<<genSTRD(ba-0x1000,0x0002,0x0005);//0x1000
			getL<<genSTRD(ba-0x1000,0x0008,0x0009);//0x1000
			getL<<genSTRD(ba-0x100,0x0000,0x0009);//0x2000
			//минус сто потому, что сдесь базовый не 1600, а 1500 как у канала E
			break;
		case N: //N
			//getL<<genSTRL(mdbsTERM[i]-0x1000,0x0000,1);//0x1000
			getL<<genSTRD(ba-0x1000,0x0002,0x0009);
			getL<<genSTRD(ba,0x0000,0x0009);//0x2000
			break;
		default:
			break;
	}
	return getL;
}

/*
 * Списки статических регистров
 */
QStringList dataServer::genStatReg(infoADC* inf){
	QStringList getL;
	for(int i=0; i<inf->adc.size();i++){
		getL<<genStatReg(mdbsADC[i],inf->adc[i]);
	}
	for(int i=0; i<inf->tah.size();i++){
		getL<<genStatReg(mdbsTAH[i],inf->tah[i]);
	}
	for(int i=0; i<inf->vir.size();i++){
		if(inf->vir[i]==0xE0) getL<<genStatReg(mdbsE[i],inf->vir[i]);
		if(inf->vir[i]==0xF0) getL<<genStatReg(mdbsF[i],inf->vir[i]);
	}
	for(int i=0; i<inf->term.size();i++){
		getL<<genStatReg(mdbsTERM[i],inf->term[i]);
	}
	return getL;
}


/*
 * Списки динамических регистров
 */
QStringList dataServer::genDinReg(infoADC* inf){
	QStringList getL;
	for(int i=0; i<inf->adc.size();i++){
		switch(inf->adc[i]){
			case 0xA0:
				getL<<genSTRD(mdbsADC[i]-0x1000,0x0001,0x0008); //0x1000
				getL<<genSTRD(mdbsADC[i]-0x1000,0x0019,0x001C);
				getL<<genSTRD(mdbsADC[i]-0x1000,0x0046,0x004C);
				if(inf->ath[i][0]){
					getL<<genSTRL(mdbsADC[i]-0x1000,0x0056,1);
					getL<<genSTRD(mdbsADC[i]-0x1000,0x005B,0x0064);
				}
				if(inf->ath[i][1]){
					getL<<genSTRL(mdbsADC[i]-0x1000,0x0058,1);
					getL<<genSTRD(mdbsADC[i]-0x1000,0x0065,0x006E);
				}
				if(inf->ath[i][2]){
					getL<<genSTRL(mdbsADC[i]-0x1000,0x005A,1);
					getL<<genSTRD(mdbsADC[i]-0x1000,0x006F,0x0078);
				}
				getL<<genSTRL(mdbsADC[i]-0x1000,0x0079,1);
				getL<<genSTRL(mdbsADC[i]-0x1000,0x0082,1);
				getL<<genSTRD(mdbsADC[i]-0x1000,0x008F,0x0090);
				getL<<genSTRL(mdbsADC[i]-0x1000,0x00A9,1);
				getL<<genSTRD(mdbsADC[i],0x00F1,0x00F8);//0x2000
				break;
			case 0xC0:
				getL<<genSTRD(mdbsADC[i]-0x1000,0x001B,0x0025);//0x1000
				if(inf->ath[i][0]){
					getL<<genSTRD(mdbsADC[i]-0x1000,0x004D,0x0053);
					getL<<genSTRL(mdbsADC[i]-0x1000,0x0056,1);
				}
				getL<<genSTRL(mdbsADC[i]-0x1000,0x008F,1);
				getL<<genSTRD(mdbsADC[i],0x00F1,0x00F4);//0x2000
				break;
			case 0xD0:
				getL<<genSTRL(mdbsADC[i]-0x1000,0x001D,1);//0x1000
				getL<<genSTRL(mdbsADC[i]-0x1000,0x008F,1);
				getL<<genSTRL(mdbsADC[i],0x00F1,1);//0x2000
				break;
			case 0x20:
				getL<<genSTRL(mdbsADC[i]-0x1000,0x001D,1);//0x1000
				getL<<genSTRL(mdbsADC[i]-0x1000,0x008F,1);
				break;
			case 0x30:
				getL<<genSTRD(mdbsADC[i]-0x1000,0x001D,0x001F);//0x1000
				getL<<genSTRL(mdbsADC[i]-0x1000,0x008F,1);
				getL<<genSTRD(mdbsADC[i],0x00F1,0x00F3);//0x2000
				break;

		}
	}
	for(int i=0; i<inf->tah.size();i++){
		if(inf->tah[i]){
			getL<<genSTRL(mdbsTAH[i]-0x1000,0x0002,1);//0x1000
			getL<<genSTRL(mdbsTAH[i]-0x1000,0x000B,1);
			getL<<genSTRL(mdbsTAH[i],0x000A,1);//0x2000
		}
	}
	for(int i=0; i<inf->vir.size();i++){
		if(inf->vir[i]==0xE0){
			getL<<genSTRL(mdbsE[i]-0x1000,0x0001,1);//0x1000
		}
		if(inf->vir[i]==0xF0){
			getL<<genSTRL(mdbsF[i]-0x1000,0x0001,1);//0x1000
			getL<<genSTRL(mdbsF[i]-0x1000,0x0007,1);

		}
		if(inf->vir[i])	 getL<<genSTRL(mdbsE[i],0x000A,1);       //0x2000 - смещение как у E !!!!
	}
	for(int i=0; i<inf->term.size();i++){
		if(inf->term[i]){
			getL<<genSTRL(mdbsTERM[i]-0x1000,0x0001,1);//0x1000
			getL<<genSTRL(mdbsTERM[i],0x000A,1);//0x2000
		}
	}


	//если каналов нет - читаем по кругу тип первого
	if(getL.size()==0) getL<<genSTRL(mdbsADC[0]-0x1000,0x0000,1);
	return getL;
}

/*
 * Списки динамических регистров адаптивный
 */
QStringList dataServer::genDinReg(infoADC* inf,QDomDocument mdoc){
	QStringList getL;
	QMap<ushort,ushort> getMap;
	QDomNodeList list;
	QDomElement ec;
	QDomNode el;
	ushort cuad;
	bool ok;

	for(int i=0; i<inf->adc.size();i++){
		QDomNode ea;
		switch(inf->adc[i]){
			case A:
				ea=mdoc.elementsByTagName("A").at(0);
				break;
			case C:
				ea=mdoc.elementsByTagName("C").at(0);
				break;
			case D:
				ea=mdoc.elementsByTagName("D").at(0);
				break;
			case M:
				ea=mdoc.elementsByTagName("M").at(0);
				break;
			case I:
				ea=mdoc.elementsByTagName("I").at(0);
				break;
		}
		if(ea.isNull()) continue;
		list=ea.childNodes();
		for(int j=0;j<list.size();j++){
			ec=list.at(j).toElement();
			if(ec.attribute("read","0")!="0"){
				cuad=ec.attribute("id","0").toUShort(&ok,16);
				if(ok && cuad>0) getMap[mdbsADC[i]+cuad]=1;
			}
		}
	}


	el=mdoc.elementsByTagName("T").at(0);
	list=el.childNodes();
	for(int i=0; i<inf->tah.size();i++){
		if(inf->tah[i]){
			for(int j=0;j<list.size();j++){
				ec=list.at(j).toElement();
				if(ec.attribute("read","0")!="0"){
					cuad=ec.attribute("id","0").toUShort(&ok,16);
					if(ok && cuad>0) getMap[mdbsTAH[i]+cuad]=1;
				}
			}
		}
	}

	el=mdoc.elementsByTagName("N").at(0);
	list=el.childNodes();
	for(int i=0; i<inf->term.size();i++){
		if(inf->term[i]){
			for(int j=0;j<list.size();j++){
				ec=list.at(j).toElement();
				if(ec.attribute("read","0")!="0"){
					cuad=ec.attribute("id","0").toUShort(&ok,16);
					if(ok && cuad>0) getMap[mdbsTERM[i]+cuad]=1;
				}
			}
		}
	}

	for(int i=0; i<inf->vir.size();i++){
		if(inf->vir[i]==0xE0){
			el=mdoc.elementsByTagName("E").at(0);
			list=el.childNodes();
			for(int j=0;j<list.size();j++){
				ec=list.at(j).toElement();
				if(ec.attribute("read","0")!="0"){
					cuad=ec.attribute("id","0").toUShort(&ok,16);
					if(ok && cuad>0) getMap[mdbsE[i]+cuad]=1;
				}
			}
		}
		if(inf->vir[i]==0xF0){
			el=mdoc.elementsByTagName("F").at(0);
			list=el.childNodes();
			for(int j=0;j<list.size();j++){
				ec=list.at(j).toElement();
				if(ec.attribute("read","0")!="0"){
					cuad=ec.attribute("id","0").toUShort(&ok,16);
					if(ok && cuad>0) getMap[mdbsF[i]+cuad]=1;
				}
			}
		}
	}

	//если каналов нет - читаем по кругу тип первого
    if(getMap.size()==0) getMap[mdbsADC[0]]=1;

	//нужно скомпоновать по группам
	QList<ushort> key=getMap.keys();//ключи по порядку
	if(key.size()==0) return getL;
	cuad=key.at(0);
	ushort sz=1;
	if(key.size()==1) 	getL<<genSTRA(cuad-0x1000,sz);
	else{
		for(int i=1;i<key.size();i++){
			if(key[i]==cuad+sz) sz++;
			else{
				getL<<genSTRA(cuad-0x1000,sz);
				sz=1;
				cuad=key[i];
			}
			if(i==key.size()-1){//последний
				getL<<genSTRA(cuad-0x1000,sz);
			}
		}
	}

	return getL;
}


/*
 * формирует список строк  данных вычитки файла из строк вида "aaaassss"
 * где aaaa- адрес в файле, ssss-размер вычитки
 */
QList<QByteArray> dataServer::fileReadArr(QStringList str){
	QList<QByteArray>  rez;
	bool ok;

	int dls=7,lse=0; //суммарные длинны запроса-ответа
	int dlr,lre=0;  //длинны текущего запроса-ответа
	int st=0;

	for(int i=0; i<str.size();i++){

		dlr=2+2*str[i].right(4).toUInt(&ok,16); //ответ

		if(lse+dls+3<=254 && lre+dlr+3<=254) {lse+=dls; lre+=dlr;}
		else{
			rez.append(QByteArray());
			rez.last().reserve(1+lse);
			rez.last().append((unsigned char)lse);
			for(int j=st;j<i;j++){
				rez.last().append(execut::convert("060001"));
				rez.last().append(execut::convert(str[j]));
			}
			lse=dls ;lre=dlr;	st=i;
		}

		if(i==str.size()-1){//последний!
			rez.append(QByteArray());
			rez.last().reserve(1+lse);
			rez.last().append((unsigned char)lse);
			for(int j=st;j<=i;j++){
				rez.last().append(execut::convert("060001"));
				rez.last().append(execut::convert(str[j]));
			}
		}

	}
	return rez;
}

/*
 *  Формирует строку вычитки
 */

QString dataServer::genSTRA(ushort a,ushort l){
	usort ad,le;
	ad.sd=a;
	le.sd=l;
	QByteArray arr(4,0);
	arr[0]=ad.sc[1];
	arr[1]=ad.sc[0];
	arr[2]=le.sc[1];
	arr[3]=le.sc[0];
	return execut::ByteArray_to_String(arr);
}


QString dataServer::genSTRL(ushort sm,ushort a1,ushort l){
	usort ad,le;
	ad.sd=sm+a1;
	le.sd=l;
	QByteArray arr(4,0);
	arr[0]=ad.sc[1];
	arr[1]=ad.sc[0];
	arr[2]=le.sc[1];
	arr[3]=le.sc[0];
	return execut::ByteArray_to_String(arr);
}

QString dataServer::genSTRD(ushort sm,ushort a1,ushort a2){
	return genSTRL(sm,a1,a2-a1+1);
}

void dataServer::stop(){
	bool st=false;
	//ThreadList запскается, останавливается и очищается внутри uThread
	if(uThread){
		uThread->stop();
		uThread->wait();
		delete(uThread);
		uThread=NULL;
		st=true;
	}

	while(snifferTList.size()>0){
		snifferTList.last()->stop();
		snifferTList.last()->wait();
		delete(snifferTList.last());
		snifferTList.last()=NULL;
		snifferTList.removeLast();
		if(snifferTList.size()==0){
			qWarning()<<"stop LISTEN client";
			st=true;
		}
	}
	mMap::const_iterator it=map->begin();
	for(;it!=map->end();it++) it.value()->status=MArray::eStop;

}


//---------------------------------------
//когда закончилось выполнение шарманки
void dataServer::exitRun(int m){

	connectType conK,conT=cEXT;
	Prot485  P485=Modbus;
	QString conNM,conSP;
	ExeListThread* thread = qobject_cast<ExeListThread*>(sender());

	if(m==0){
		thread->restartExe(0);
		return;
	}
	MArray* dm=map->value(m);

	if(dm->status==MArray::eStop) return;

	command* com=dm->exe->getTape()->at(0);
	if(com->getType()==CONNECT){
		conK=conT=((dCONNECT*)com)->cType;
		P485=((dCONNECT*)com)->cProt485;
	}
	if(conT==cEXT && dm->exe->extExe!=NULL){
			com=dm->exe->extExe->getTape()->at(0);
			conT=((dCONNECT*)com)->cType;
    }
	switch(conT){
		case cUDP:
		case cTCP:
		case cCOM:
			conNM=((dCONNECT*)com)->cAddr;
			conSP=((dCONNECT*)com)->cParam;
			break;
		case cEXT:
			break;
	}
	int n;//номер в потоке
	for(n=0;n<thread->lexe.size();n++){
		if(thread->lexe[n]->getTape()->marker==m) break;
	}

	//QString tm;
	switch(dm->status){
		case MArray::eInfo:
			getMINFO(dm); //из модбас регистров получили данные о конфигурации
			dm->status= MArray::eData;
			dm->dstatus=false;
			switch(conT){
				//case cEXT:
				case cUDP:
					thread->restartExe(n,exe_SD(m,conK,conNM,conSP,mUDP[conNM]));
					break;
				case cTCP:
					thread->restartExe(n,exe_SD(m,conK,conNM,conSP,mTCP[conNM]));
					break;
				case cCOM:
					if(P485==Modbus) thread->restartExe(n,exe_SD(m,conK,conNM,conSP,mCOM[conNM]));
					else             thread->restartExe(n,exe_DV(m,conK,conNM,conSP,mCOM[conNM]));
					break;
			}
			qWarning()<<m<<"get  DATE:"+conNM+":"+conSP;
			break;
		case  MArray::eData:
			dm->status= MArray::eInfo;
			dm->dstatus=false;
			switch(conT){
				//case cEXT:
				case cUDP:
					thread->restartExe(n,exe_I(m,conK,conNM,conSP,mUDP[conNM]));
					break;
				case cTCP:
					thread->restartExe(n,exe_I(m,conK,conNM,conSP,mTCP[conNM]));
					break;
				case cCOM:
					if(P485==Modbus) thread->restartExe(n,exe_I(m,conK,conNM,conSP,mCOM[conNM]));
					else             thread->restartExe(n,exe_SV(m,conK,conNM,conSP,mCOM[conNM]));
					break;
			}
			qWarning()<<m<<"wait DATE:"+conNM+":"+conSP;
			break;
		case MArray::eStop:
			break;
	}
}




