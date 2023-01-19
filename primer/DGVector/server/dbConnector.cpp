/*
 * dbConnector.cpp
 *
 *  Created on: 20 окт. 2014 г.
 *      Author: Usach
 */

#include "dbConnector.h"


dbConnector::dbConnector(const QString& Name, QObject *parent) : QObject(parent) {

	if(Name!="") nameCon=Name;
	else         nameCon="dafault";
	stat=0;

}

dbConnector::~dbConnector() {
	dbClose();
}

void dbConnector::dbClose(){

	if(QSqlDatabase::contains(nameCon)){
		QSqlDatabase db(QSqlDatabase::database(nameCon));
		if(db.isOpen()){
			db.close();
		    qWarning()<<"Data base disconnected: "+nameCon;
		}
		stat=0;
    }
	if(QSqlDatabase::contains(nameCon)) QSqlDatabase::removeDatabase(nameCon);
}

bool dbConnector::dbConnect(QDomElement sElem,QString& dbType){

	stat=0;
	if(sElem.isNull())            return false;
	if(sElem.nodeName()!="dbsrv") return false;
	set=sElem;
	bool ok;
	int port;

	QString hostName, dbName,user,passwd;
	QString type=set.attribute("type","NONE");
	hostName=set.attribute("ipaddr","127.0.0.1");
	port=set.attribute("port","0").toInt(&ok);
	dbName=set.attribute("base","test");
	user=set.attribute("user","");
	passwd=set.attribute("passwd","");
	if(dbName.trimmed()=="") dbName="test";
	dbType.clear();
	if(type=="MySQL"){
			dbType.append("QMYSQL");
			if(port==0) port=3306;
	}else if(type=="PostgreSQL"){
			dbType.append("QPSQL");
			if(port==0) port=5432;
	}

	if(dbType=="") return false;

	QSqlDatabase db;

	dbClose();
	if(QSqlDatabase::contains(nameCon)) db=QSqlDatabase::database(nameCon);
	else                                db=QSqlDatabase::addDatabase(dbType,nameCon);
	db.setHostName(hostName);
	db.setPort(port);
	db.setDatabaseName(dbName);
	db.setUserName(user);
	db.setPassword(passwd);
	if(db.open()) {
		qWarning()<<"Data base connected: "+nameCon;
	}else{
		qDebug()<<db.lastError().text()<<"Number error:"<<db.lastError().number();

		if( (dbType=="QMYSQL" && db.lastError().number()==1049) ||
		    (dbType=="QPSQL" && db.lastError().number()==-1) 	  ){

			qWarning()<<"None database: '"+dbName+"'";
			db.setDatabaseName("");
			if(db.open() && db.exec("CREATE DATABASE "+dbName+";").lastError().type()==0){
				    qDebug()<<"Create database '"+dbName+"'";
					db.close();
					db.setDatabaseName(dbName);
					if(!db.open()) {
						dbClose();
						stat=-1;
						return false;
					}

			}else{
				qDebug()<<db.lastError().text();

				dbClose();
				stat=-1;
				return false;
			}
		}else{
			dbClose();
			stat=-1;
			return false;
		}
	}
	if(true) stat=1;
	return true;
}



