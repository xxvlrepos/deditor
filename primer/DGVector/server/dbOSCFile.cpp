/*
 * dbOSCFile.cpp
 *
 *  Created on: 31 окт. 2014 г.
 *      Author: Usach
 */

#include "dbOSCFile.h"



//**************************************************************************************

dbOSCFile::dbOSCFile(const QString& Name,QString& type, QObject *parent) : QObject(parent) {
	nameConn=Name;
	lifeTime=0;
	if(type=="QMYSQL"){
		tableTOSC=new TableTOSC_M();
	}else if(type=="QPSQL"){
		tableTOSC=new TableTOSC_P();
	}
	thread=NULL;
}

dbOSCFile::~dbOSCFile() {
	stop();
	deleteThred();
	delete(tableTOSC);
}

bool dbOSCFile::dbConnect(QDomElement sElem,int lt){

	lifeTime=lt;
	parConn=sElem;

//	dbDataThread* thread;

	thread=new dbDataThread(this);
	thread->moveToThread(thread);
	connect(this, SIGNAL(sendF(QString)), thread, SLOT(sendF(QString)));
	connect(this, SIGNAL(stopAll()), thread, SLOT(quit()));
	connect(thread, SIGNAL(finished()), this, SLOT(deleteThred()));
    thread->start();
	return true;
}

void dbOSCFile::iniTable(){
	if(QSqlDatabase::contains(nameConn)){
	QSqlDatabase db(QSqlDatabase::database(nameConn));
		if(db.open()){

		 QSqlQuery q(db);
		// Создаем таблицу осцилограмм. Если таблица существует создавать вторую не будет...
		if (q.exec(tableTOSC->greate())) qDebug()<<"Create new database table '"+tableTOSC->Name+"'";
		}
	}
}

void dbOSCFile::FileToDB(QString fName){

	if(!QSqlDatabase::contains(nameConn)) return;
	QSqlDatabase db(QSqlDatabase::database(nameConn));
	if(!db.open()) return;

	QByteArray  fDat;
	QString sName=fName.section('/',-1);
	QStringList fAtr=sName.split(QRegExp("(_)|(\\.)"));
	if(fAtr.size()!=4) return;
	QFile oscF(fName);
	if(oscF.open(QIODevice::ReadOnly)){
		fDat=oscF.readAll();
	}else{
		qWarning()<<"Not load file: "+sName;
		return;

	}

	 QSqlQuery q(db);
	 q.prepare("INSERT INTO "+tableTOSC->Name+" VALUES (:utime, :modbus, :chanel, :dataosc);");
	 q.bindValue(":utime",fAtr[0]);
	 q.bindValue(":modbus",fAtr[1]);
	 q.bindValue(":chanel",fAtr[2]);
	 q.bindValue(":dataosc", fDat,QSql::In | QSql::Binary);
	 q.exec();


	 if(lifeTime>0){
		 uint time=fAtr[0].toUInt();
		 QString stime;
		 QSqlQuery ql(db);
		 ql.exec("DELETE FROM "+tableTOSC->Name+" WHERE utime < "+stime.setNum(time-lifeTime*3600)+";");
	 }
}

void dbOSCFile::deleteThred(){

 	if(thread){
 		thread->wait();
 		delete(thread);
 		thread=NULL;
 	}
 }

void dbOSCFile::addFileOSC(QString fName){
	emit sendF(fName);
}


//***********************************************************************

dbDataThread::dbDataThread(dbOSCFile* dbOSC, QObject *parent): QThread(parent){
	dc=dbOSC;
}

void dbDataThread::run(){
	QString dbType;
	dcon=new dbConnector(dc->getNameConn());
	if(!dcon->dbConnect(dc->getParConn(),dbType)) return;
	dc->iniTable();

	timer=new QTimer();
	timer->setSingleShot(true);
	connect(timer, SIGNAL(timeout()),this,SLOT(send()));
	exec();
	dcon->dbClose();
}

void dbDataThread::sendF(QString Name){
	mutex.lock();
	   fileQueue.enqueue(Name);
	mutex.unlock();
	timer->start(0);
}

void dbDataThread::send(){
	QString Name;
	int sz;
	while(1){
		 mutex.lock();
		 	sz=fileQueue.size();
			if(sz>0) Name=fileQueue.dequeue();
		 mutex.unlock();
		 if(sz==0) break;
		 dc->FileToDB(Name);
	}
}


