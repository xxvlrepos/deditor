/*
 * oscCollector.cpp
 *
 *  Created on: 02 окт. 2014 г.
 *      Author: Usach
 */

#include "oscCollector.h"

#if QT_VERSION >= 0x050000
#include  <QtConcurrent/QtConcurrent>
#endif


   const QStringList oscCollector::frList=IniQStringList()<<"32000"<<"16000"<<"8000"<<"4000";
   const QStringList oscCollector::resList=IniQStringList()<<"0.25"<<"0.5"<<"1"<<"2"<<"4";

oscCollector::oscCollector(mMap* m,int timOu, QObject *parent): QObject(parent) {

	map=m;
	fDir=&XSD::appPath;
	TimeOut=timOu;
	run=false;
	diskFileSearch=true;
	testLT.setTimeSpec(Qt::LocalTime);
	shedJob=new Sheduler(this);
	connect(shedJob,SIGNAL(startJob(int)),this,SLOT(runJob(int)));
}

oscCollector::~oscCollector() {
	// TODO Auto-generated destructor stub
	stopAll();
	while(TredList.size()>0){
		deleteThred(TredList.last());
	}
}

void oscCollector::stopAll(){
	 shedJob->clear();
	 jobList.clear();
	 if(work) qWarning()<<"stop OSC collector";
	 work=false;
	 emit stopAllThread();
}

bool oscCollector::start(QDomElement sElem){

	if(sElem.isNull())          return false;
	if(sElem.nodeName()!="osc") return false;
	osc=sElem;


	bool ok;
	bool rez=false;

	shedJob->clear();
	jobList.clear();
	//statusList.clear();
	fileOSCMap.clear();
	diskFileSearch=true;

	fileDir=fDir->mid(0)+"/OSC";
	lifeTime=osc.attribute("lifetime","24").toInt(&ok,10);
	repeatOSC=osc.attribute("repeatOSC","5").toInt(&ok,10);

	QDomNodeList nl=osc.childNodes();
	QDomElement job;
	map->countShed=nl.size();
	statusList.resize(nl.size());
	for(int i=0;i<nl.size();i++){
		job=nl.at(i).toElement();
		QString devises,minutes,hours;
		statusList[i]=-1;

		devises=job.attribute("devises","0");
		minutes=job.attribute("minutes");
		hours=job.attribute("hours");

		if(devises!="0"){
			QStringList sList=devises.split(",",QString::SkipEmptyParts);
			Job cJob;
			cJob.resolution=job.attribute("resolution","0.25");
			cJob.frequency=job.attribute("frequency","32000");
			cJob.MultJobClose=(job.attribute("multiJobClose","0")!="0");
			for(int j=0;j<sList.size();j++){
				unsigned char m=0;
				m=(unsigned char)sList.at(j).toUShort(&ok);
				if(ok && m>0 && map->contains(m)) cJob.devList<<m;
			}

			QString ts,adr; //тип соед и адрес
			for (int j=0; j<cJob.devList.size(); j++) {
				if (j==0) { //проверка на общее соединение
					ts= map->value(cJob.devList[j])->tipCon;
					if (ts=="COM485"){
						if(map->value(cJob.devList[j])->type!=MArray::kNONE){
							cJob.devList.clear();
							qDebug()<<"job N"<<i+1<<" No Modbus protocol!";
							break;
						}
							adr = map->value(cJob.devList[j])->comaddr;
					}
					else 	adr = map->value(cJob.devList[j])->ip;
				}else{
					if(map->value(cJob.devList[j])->tipCon != ts) {
						cJob.devList.clear();
						break;
					}
					if(ts=="TCP") {
						if (map->value(cJob.devList[j])->ip != adr) {
							cJob.devList.clear();
							break;
						}
					}
				}
			}
			if(cJob.devList.size()>0){
				if(shedJob->append(minutes,hours)){
					cJob.id=i;
					jobList<<cJob;
					statusList[i]=0;
					qDebug()<<"job N"<<i+1<<" ok!";
				}else qDebug()<<"job N"<<i+1<<" error dev shedule";
			}else{
				qDebug()<<"job N"<<i+1<<" error dev list";
			}
		}else  qDebug()<<"job N"<<i+1<<" disabled";

	}
	if(jobList.size()>0){
		rez=true;
		work=true;

		shedJob->start();
	}
	return rez;
}

void oscCollector::runJob(int n){

   int err=0;

   if(n>=jobList.size()) return;
   int rez= getOSC(jobList[n],err,jobList[n].id);
   if(rez) statusList[jobList[n].id]=1;
   qWarning()<<"job N:"<<jobList[n].id+1<<" run:"<<rez<<" err:"<<err;
   if(rez==1) run=true;
}


int oscCollector::getOSC(Job& job, int& err,int id){

	const QList<unsigned char>& sp=job.devList;

	int Dec=1;
	int Coup=maxCount;
	int Krs=1;

	if(frList.contains(job.frequency)) Dec=qPow(2,frList.indexOf(job.frequency));
	if(resList.contains(job.resolution)) Krs=qPow(2,resList.indexOf(job.resolution));
	Coup=maxCount/(Dec*Krs);

	err=0;

	tape* tap;
	unsigned int  Hesh=qrand()*QTime::currentTime().msec();
	QMap<unsigned char,QList<unsigned char> > spp; // проверенный список адресов и каналов
	runJobId spM; //проверенный список адресов
	//начало проверки
	QString ts,adr; //тип соед и адрес
	for(int i=0;i<sp.size();i++){
		if(map->contains(sp[i])){ //проверка на наличие
			if(i==0) {//проверка на общее соединение
				ts=map->value(sp[i])->tipCon;
				if(ts=="COM485") adr=map->value(sp[i])->comaddr;
				else             adr=map->value(sp[i])->ip;
			}else{
				if(map->value(sp[i])->tipCon!=ts)  {err|=0x16; return 0;}
				if(ts=="TCP"){
					if(map->value(sp[i])->ip!=adr) {err|=0x16; return 0;}
				}
			}
			if(map->value(sp[i])->status==MArray::eData){//проверка на состояние (опрашивается)
				if(!testRun(sp[i])){
					if(map->value(sp[i])->inf.adc.size()>0){
						spp[sp[i]]=QList<unsigned char>();
						spM.devList<<sp[i];
						for(int j=0;j<map->value(sp[i])->inf.adc.size();j++){
							if(map->value(sp[i])->inf.adc.at(j)!=0) spp[sp[i]]<<(j+1);
						}
					}else err|=0x8;//нет вкл каналов
				}else err|=0x4;  //уже опрашивается
			}else err|=0x2; //прибор не опрашивается (недоступен)
		}else err|=0x1; //одного или > приборов нет в списке опроса
	}

	if(spp.size()==0) return 0;
	//генерируем листинг шарманки опроса

	if(!QDir().mkpath(fileDir)) return 0;
	tap= exe_OSC(fileDir,spp,Hesh,job.MultJobClose,Dec,Coup);
	tap->setMaxRepeat(repeatOSC);//число повторов

	//tap->test(true);

	spM.id=id;
	ExeThread *thread;
	thread=new ExeThread(tap);
	runList[(quint64)thread]=spM; //что опрашиваем


	connect(this, SIGNAL(stopAllThread()),  thread, SLOT(halt()));
	connect(thread, SIGNAL(finished()), this, SLOT(deleteThred()));
	connect(thread, SIGNAL(fileLoaded(QString)), this, SLOT(endLoadF(QString)));
	TredList<<thread;

	thread->moveToThread(thread);
	thread->start();

	return 1;
}

void oscCollector::endLoadF(QString fn){
	qDebug()<<"get OSC file: "+fn.section('/',-1);
	fileOSCMap[fn.section('/',-1)]=QDateTime::currentDateTime();
	emit fileLoaded(fn);
}

bool oscCollector::testRun(unsigned char m){

	QHash<quint64,runJobId >::iterator it;
	for (it = runList.begin(); it != runList.end(); ++it){
		if(it.value().devList.contains(m)) return true;
	}
	return false;
}

void oscCollector::deleteThred(){

	ExeThread* thread = qobject_cast<ExeThread*>(sender());
	deleteThred(thread);
 }

void oscCollector::deleteThred(ExeThread* thread){

	int id=runList[(quint64)thread].id;
	runList.remove((quint64)thread);
	statusList[id]=0;
	qWarning()<<"job N:"<<id+1<<" end";
	thread->wait();
	TredList.removeAt(TredList.indexOf(thread));
	delete(thread);
	thread=NULL;
	run=false;

 	if(work){
 		if(!futureClearF.isRunning()){
 			testLT=QDateTime::currentDateTime().addSecs(-3600*lifeTime);
 			futureClearF=QtConcurrent::run(this,&oscCollector::clearOldFile,fileOSCMap,fileDir,testLT,diskFileSearch);
 		}
 	}
 	if(TredList.size()==0)	futureClearF.waitForFinished();
 	//qDebug()<<"del tr!!!";
 }

void oscCollector::clearOldFile(QMap<QString,QDateTime>& fileOSCMap,QString& fileDir,QDateTime& testLT,bool& discSearch){


	QStringList delFile;
	if(discSearch){
		QFileInfoList fil=QDir(fileDir).entryInfoList(QDir::Files,QDir::Time);
		for(int i=0; i<fil.size();i++){
			fileOSCMap[fil.at(i).fileName()]=fil.at(i).created();
		}
		discSearch=false;
	}
	QMap<QString,QDateTime>::iterator it=fileOSCMap.begin();
	for(;it!=fileOSCMap.end();it++) {
		if(it.value()<testLT) delFile<<it.key();
	}
	int cou=0;
	for(int i=0; i<delFile.size();i++){
		if(QFile::remove(fileDir+"/"+delFile[i])){
			cou++;
			fileOSCMap.remove(delFile[i]);
		}
	}
	if(cou>0) qWarning()<<"Delete"<<cou<<" old files";
}

tape* oscCollector::exe_OSC(QString dir,QMap<unsigned char,QList<unsigned char> > &sp, unsigned int h,bool multi, unsigned char d, unsigned int cou){


	tape* tp=new tape();

	bool variableCon=false;  //тип соединения разнородный?
	QByteArray heshA=execut::convert(h,4);
	QByteArray couA =execut::convert(cou,4);

	dCONNECT *con;
	dIF* di;
	dDOWNLOAD* dd;
	dBROADCAST* db;
	dMULTICAST* dm;

	QString txt,txta;
	unsigned char endAdr=0;

	execut::transport oldtransp=execut::NONE;
	execut::transport transp=execut::NONE;
	QString param;
	QHostAddress braddr=QHostAddress(QHostAddress::Broadcast);

	QMap<unsigned char,QList<unsigned char> >::iterator itsp=sp.begin();
	for(;itsp!=sp.end();itsp++){

		con=(dCONNECT*)tp->append(CONNECT);
		con->cType=cEXT;
		con->extExe=map->value(itsp.key())->exe;

		if(con->extExe->getTransport()!=execut::NONE){
			transp=(execut::transport)con->extExe->getTransport();
			if(!variableCon) variableCon=(oldtransp!=execut::NONE && oldtransp!=transp);
			oldtransp=transp;
			param =con->extExe->getPort();
		}

		//закладка группы
		di =(dIF*)tp->append(IF);
		di->setAdr(itsp.key());
		di->setFun(0x65);
		di->append(0x02);  //команда
		di->append(heshA);
		di->wait=TimeOut;

		di->addTest(di->getAF());
		di->addTest(heshA,2);
		di->addTest(execut::convert("00000000"),6);
		di->addTest(execut::convert("01"),10);

		endAdr=itsp.key();

	//---------------------------------------------------------
	}



	if(variableCon){//индивидуально закрываем, так как разные типы
		itsp=sp.begin();
		for(;itsp!=sp.end();itsp++){
			con=(dCONNECT*)tp->append(CONNECT);
			con->cType=cEXT;
			con->extExe=map->value(itsp.key())->exe;

			di =(dIF*)tp->append(IF);
			di->setAdr(itsp.key());
			di->setFun(0x65);
			di->append((char)0x0A);        //команда
			di->append(heshA);             //имя
		}

	}else{//тип соединения однородный
		if(transp==execut::UDP){//останов записи-используем отдельное соединение для броадкаста
			con=(dCONNECT*)tp->append(CONNECT);
			con->cType=cUDP;
			con->cAddr=braddr.toString();
			con->cParam=param;
		}

		if(multi && transp==execut::UDP){//закрываем мультикастом

			dm=(dMULTICAST*)tp->append(MULTICAST);	//добавили команду
			itsp=sp.begin();
			for(;itsp!=sp.end();itsp++){ //список запросов
				dm->appendAdr(map->value(itsp.key())->exe->getAddress(),itsp.key());
			}
			dm->setAdr(0x00);              //адрес меняем по списку
			dm->setFun(0x65);
			dm->append((char)0x0A);        //команда
			dm->append(heshA);             //имя
			dm->after_delay_OK=500;  //ждем ответы, но не разбираем

		}else{//закрываем броадкастом
			db =(dBROADCAST*)tp->append(BROADCAST);	//добавили команду
			db->setAdr(0x00);               //броадкаст
			db->setFun(0x65);
			db->append((char)0x0A);        //команда
			db->append(heshA);             //имя
			db->after_delay_OK=500;
			db->after_delay_BAD=500;
		}


	}

//	tp->append(DISCONNECT);

	//выкачка файла
	itsp=sp.begin();
	int n=0;
	//----------------------------------------------------
	for(;itsp!=sp.end();itsp++){

		n++;
		con=(dCONNECT*)tp->append(CONNECT,"CON"+txt.setNum(n));
		con->cType=cEXT;
		con->extExe=map->value(itsp.key())->exe;

		//контроль готовности
		di =(dIF*)tp->append(IF);
		di->setAdr(itsp.key());
		di->setFun(0x65);
		di->append(0x01);  //команда
		di->wait=TimeOut;
		if(n==1) di->delay=100;
		di->ControlRez=true;// излучать результат

		di->addTest(di->getAF());
		di->addTest(heshA,2);
		if(endAdr!=itsp.key()) di->goBAD_L="CON"+txt.setNum(n+1);
		else di->goBAD=-1;


		for(int i=0;i<itsp.value().size();i++){
			dd=(dDOWNLOAD*)tp->append(DOWNLOAD);
			dd->setAdr(itsp.key()); //адрес
			dd->setFun(0x65);
			dd->wait=TimeOut;
			dd->ControlRez=true;// излучать результат

			QByteArray cmd; cmd.reserve(12);
			cmd.append(heshA);
			cmd.append(itsp.value().at(i));
			cmd.append(d);
			cmd.append(couA);
			cmd.append(execut::convert("0000"));

			dd->setCmd(cmd);
			dd->addTest(dd->getAF());

			dd->dirName=dir;
			txt.setNum(itsp.value().at(i));
			txta.setNum(itsp.key());
			dd->afterName="_"+txta+"_"+txt+".BIN";
			if(endAdr!=itsp.key()) dd->goBAD_L="CON"+txt.setNum(n+1);
			else dd->goBAD=-1;
		}
	}

	tp->append(DISCONNECT);

	return tp;
}

