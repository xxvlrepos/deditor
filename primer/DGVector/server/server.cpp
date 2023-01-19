/*
 * server.cpp
 *
 *  Created on: 04 дек. 2014 г.
 *      Author: Usach
 */

#include "server.h"
#include "../qtservice/src/qtservice.h"

//******************************
mMap MDBArray;
//******************************


server* server::MyServer=NULL;
QString server::ver="3.1.4";
QString  server::nsvn="1214";

//******************************

server::server(mMap* m,QObject *parent): QObject(parent) {


	map=m;
	dServer=NULL;
	mServer=NULL;
	oscColl=NULL;
	sdiagColl=NULL;
	dbOSC=NULL;
	dbColl=NULL;
	uaServer=NULL;
    conDelay=conCou=couDBCon=0;
	run=false;
	xmlName=XSD::appPath+"/"+XSD::SettingsXML;
	xsdName=XSD::SettingsXSD;
    MyServer=this;



    xmlRPC=new MaiaXmlRpcServer(RPC::Port,this);
    xmlRPC->debugC=SYSTEM::Debug;
    if(xmlRPC->isListening()){
    		qWarning()<<"start RPC server to: "+QString::number(RPC::Port);
    }

    xmlRPC->addMethod("server.start",  this,"startRPC");
    xmlRPC->addMethod("server.stop",   this,"stopRPC");
    xmlRPC->addMethod("server.isRun",  this,"isRun");
    xmlRPC->addMethod("server.getVer",  this,"getVer");
    xmlRPC->addMethod("server.getStatusV",        this,"getStatusV");
    xmlRPC->addMethod("server.getStatusOSCColV",  this,"getStatusOSCColV");
    xmlRPC->addMethod("server.getStatusDB",       this,"getStatusDB");
    xmlRPC->addMethod("server.getLogTxt",         this,"getLogTxt");
    xmlRPC->addMethod("server.getSettings",       this,"getSettings");
    xmlRPC->addMethod("server.getModbusSett",     this,"getModbusSett");
    xmlRPC->addMethod("server.setSettings",       this,"setSettings");
    xmlRPC->addMethod("server.setModbusSett",     this,"setModbusSett");

    testFile(XSD::appPath+"/"+XSD::SettingsXML,":/hml/resource",XSD::SettingsXSD);
    testFile(XSD::appPath+"/"+XSD::ModbusXML,":/hml/resource",XSD::ModbusXSD);


}



server::~server() {
	stop();
	if(xmlRPC) xmlRPC->deleteLater();
	MyServer=NULL;
}

void server::restart(QString nm){
	    if(nm=="") nm=xmlName;
		stop();
		start(nm);
}

/*
void server::dstart(int delay,int cou){

    if(delay>0) conDelay=delay;
    if(cou>0)   conCou=cou;

	QString type;
	bool dbOk=true;
    if(xmldoc.isNull()){
        if(!XSD::xmlValidator(XSD::SettingsXSD,XSD::appPath+"/"+XSD::SettingsXML,&xmldoc)) return;
    }
    QDomElement DBset=xmldoc.documentElement().firstChildElement("dbsrv");

	if(!DBset.isNull()){
		    type=DBset.attribute("type","NONE");
			if(type!="NONE") 	dbOk=testDBService(DBset,dbType);
	}
	if(dbOk){
        conDelay=conCou=couDBCon=0;
        QTimer::singleShot(5000,this, SLOT(start()));
    }else{
		couDBCon++;
        if(couDBCon>conCou){
            conDelay=conCou=0;
            QTimer::singleShot(0,this, SLOT(start()));
        }else{
            qDebug()<<"Wait start '"+type+"' service " +QString::number(couDBCon*conDelay)+ " msec...";
           QTimer::singleShot(conDelay,this, SLOT(dstart()));
		}
	}
}
*/

bool server::start(const QString& Name,const QString& xName){

	xsdName=xName;
	if(!XSD::xmlValidator(xsdName,Name,&xmldoc)) return false;
	xmlName=Name;

	rootNode=xmldoc.documentElement();
	map->clear();//зачистили хранилище


	//настройки модулей
	QDomElement dServerSet=rootNode.firstChildElement("client");
	QDomElement mServerSet=rootNode.firstChildElement("server");
	QDomElement oscCollSet=rootNode.firstChildElement("osc");
	QDomElement sdiagCollSet=rootNode.firstChildElement("sdiag");
	QDomElement DBset     =rootNode.firstChildElement("dbsrv");
	QDomElement OpcUaServerSet=rootNode.firstChildElement("opc_ua");

	XSD::xmlValidator(XSD::ModbusXSD,XSD::appPath+"/modbus.xml",&mdbXmlDoc);

	if(!dServerSet.isNull()){
		if(!dServer){
			dServer=new dataServer(map,this);
			run=dServer->start(dServerSet,mdbXmlDoc,!mServerSet.isNull());//нет м-сервера - не нужно зеркало полное
			if(!run){
				qCritical()<<"device is not configured!";
				delete(dServer);
				dServer=NULL;
				return false;
			}
		}
	}else{
		qCritical()<<"device is not configured!";
		return false;
	}

	if(!OpcUaServerSet.isNull()){//берем те-же настройки временно
		if(!uaServer){
			uaServer=new opcuaServer(map,mdbXmlDoc,this);
			connect(dServer, SIGNAL(finDin(int)),  uaServer, SIGNAL(genData(int))); //переизлучает
			uaServer->start(OpcUaServerSet);
		}
	}


	if(!mServerSet.isNull()){
		if(!mServer){
			mServer=new mdbServer(map,this);
			mServer->start(mServerSet);
		}
	}

	if(!oscCollSet.isNull()){
		if(!oscColl){
			oscColl=new oscCollector(map,dServer->getTimeOut(),this);
			oscColl->start(oscCollSet);
		}
	}

	bool  dbOk=false;//testDBService(DBset,dbType);
	if(!DBset.isNull()){
		if(DBset.attribute("type","NONE")!="NONE"){

            if(couDBCon==0){
                dbConnector dbInit("dbInit");
                dbOk=dbInit.dbConnect(DBset,dbType);
                dbInit.dbClose();
            }


            if(oscColl && dbOk){
				if(!dbOSC) {
					dbOSC  =new dbOSCFile("oscToDB",dbType,this);
					connect(oscColl, SIGNAL(fileLoaded(QString)), dbOSC,  SLOT(addFileOSC(QString)));
					dbOSC->dbConnect(DBset,oscColl->getLifeTime());
				}
			}
            if(DBset.attribute("histdata","0")!="0" && dbOk){
				if(!dbColl){
					dbColl=new dataCollector("dataToDB",map,mdbXmlDoc,dbType,this);
					connect(dServer, SIGNAL(finStat(int)), dbColl, SLOT(genStatic(int)));
					connect(dServer, SIGNAL(finDin(int)),  dbColl, SLOT(genDinamic(int)));
					dbColl->dbConnect(DBset);
					dbColl->start();
				}
			}



		}
	}

	 if(!sdiagCollSet.isNull()){
		QDomElement dbs;
		if(dbOk)    dbs=DBset;
		sdiagColl=new sdiagCollector(map,dbs,dbType,this);
		connect(dServer, SIGNAL(finStat(int)), sdiagColl, SLOT(iniDist(int)));
		sdiagColl->start(sdiagCollSet);
	}


	 return true;
}

void server::stop(){


	xmldoc.clear();

	rootNode=QDomElement();
	couDBCon=0;




	if(mServer)   mServer->stop();
	if(dServer)   dServer->stop();
	if(uaServer)  uaServer->stop();

	if(dbOSC)     dbOSC->stop();
	if(dbColl)    dbColl->stop();
	if(oscColl)   oscColl->stopAll();
	if(sdiagColl) sdiagColl->stopAll();


	run=false;

	if(dbOSC)     {dbOSC->deleteLater();     dbOSC=NULL;}
	if(dbColl)    {dbColl->deleteLater();    dbColl=NULL;}
	if(oscColl)   {oscColl->deleteLater();   oscColl=NULL;}
	if(sdiagColl) {sdiagColl->deleteLater(); sdiagColl=NULL;}

	if(uaServer)  {uaServer->deleteLater();  uaServer=NULL;}
	if(dServer)   {dServer->deleteLater();   dServer=NULL;}
	if(mServer)   {mServer->deleteLater();   mServer=NULL;}



}



/*
bool server::testDBService(QDomElement set,QString& dbType){
    QString hostName;
	QString type=set.attribute("type","NONE");
	hostName=set.attribute("ipaddr","127.0.0.1");
	dbType.clear();
	if(type=="MySQL") 	        dbType.append("QMYSQL");
	else if(type=="PostgreSQL")	dbType.append("QPSQL");
	else dbType="";

	if(dbType=="") return false;
	if(hostName=="127.0.0.1" || hostName.toLower()=="localhost"){
		 QtServiceController controller(type);
		 if(!controller.isInstalled()){
			 qDebug()<<"Is service '"+type+"' to localhost not installing!";
			 return false;
		 }
		 if(!controller.isRunning()){
			 qDebug()<<"Is service '"+type+"' to localhost not running!";
			 return false;
		 }else{
			 qDebug()<<"Is service '"+type+"' to localhost is running!";
		 }
	}
	return true;
}
*/

QVector<int> server::getStatusOSCCol(){
	if(oscColl) return oscColl->getStatusShed();
	else {
		return QVector<int>(map->countShed,0);
	}
}

QVariantList server::getStatusOSCColV(){
		QVariantList vl;
		QVector<int> v=getStatusOSCCol();
		for(int i=0;i<v.size();i++) vl.append(QVariant(v[i]));
		return vl;
}

QVariantMap server::getStatusV(){
		QVariantMap rez;
		QMap<unsigned char,int> st= map->getStatus();
		QMap<unsigned char,int>::const_iterator im=st.begin();
		for(;im!=st.end();im++)	rez[map->value(im.key())->id]=QVariant(im.value());
		return rez;
}

int server::getStatusDB(){

	if(!isRun()) return 0;
	else{
		if(dbColl ) {if(dbColl->getStatus()) return 1;}
		if(dbOSC)   {if(dbOSC->getStatus())  return 1;}
	}
	return -1;
}

QStringList  server::getLogTxt(QDateTime dt){
	if(logFile::logF==NULL) return QStringList();
	return logFile::logF->getLog(dt);
}

QByteArray   server::getSettings(){
	if(!run) XSD::xmlValidator(xsdName,xmlName,&xmldoc);
	return xmldoc.toByteArray();
}


QByteArray   server::getModbusSett(){
	QDomDocument xmlM;
	XSD::xmlValidator(XSD::ModbusXSD,XSD::appPath+"/modbus.xml",&xmlM);
	return xmlM.toByteArray();
}


bool server::setSettings(QByteArray doc){

	QString sName(XSD::appPath+"/"+XSD::SettingsXML);
	QFile file(sName);
	if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
		qDebug() <<"Not open write file:"+sName;
		return false;
	}else xmlName=sName;
	QTextStream out(&file);
	out.setCodec("UTF-8");
	out<<QString::fromUtf8(doc.data());
	out.flush();
	file.close();
	return true;
}

bool server::setModbusSett(QByteArray doc){
	QString xmlName(XSD::appPath+"/"+XSD::ModbusXML);
	QFile file(xmlName);
	if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
		qDebug() <<"Not open write file:"+xmlName;
		return false;
	}
	QTextStream out(&file);
	out.setCodec("UTF-8");
	out<<QString::fromUtf8(doc.data());
	out.flush();
	file.close();
	return true;
}

bool server::fileSave(const QString& xmlName,const QDomDocument& xdoc){
	QFile file(xmlName);
	if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
		qDebug() <<"Not open write file:"+xmlName;
		return false;
	}
	QTextStream out(&file);
	out.setCodec("UTF-8");
	out<<xdoc.toString();
	file.close();
	return true;
}


void server::testFile(QString Name,QString srcDir,QString xsdName){
  QDomDocument xdoc;
  bool rez;
  XSD::createFile(Name,srcDir);
  rez=XSD::xmlValidator(xsdName,Name,&xdoc);
  if(!rez){
		QFile(Name).rename(Name+".bad");
		XSD::createFile(Name,srcDir);
		XSD::xmlValidator(xsdName,Name,&xdoc);
  }
	if(xdoc.documentElement().attribute("ver")!=server::ver){
		xdoc.documentElement().setAttribute("ver",server::ver);
		fileSave(Name,xdoc);
	}
}



