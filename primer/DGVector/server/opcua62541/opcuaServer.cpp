/*
 * opcuaServer.cpp
 *
 *  Created on: 15 марта 2019 г.
 *      Author: Usach
 */

#include "opcuaServer.h"
#include <qopen62541utils.h>
#include <qopen62541valueconverter.h>
#include "dataServer.h"

//*************************************************************
void opcThread::run(){
		opsua=new UA(map,srv);
		connect(srv,SIGNAL(genData(int)),opsua,SLOT(genData(int)));
		connect(this,SIGNAL(stop()),opsua,SLOT(stop()));//корректно завершаем
		connect(opsua,SIGNAL(isStoped()),this,SLOT(quit()));//остановили
	    exec();//по факту запускаем сервер
	};

//*******************************************************************

UA::UA(mMap* m, opcuaServer *sr, QObject *parent) : QObject(parent){

	map=m;
	ser=sr;
	rootNode=ser->xmldoc.documentElement();
	QDomNodeList list=rootNode.childNodes();
	for (int i=0;i<list.size();i++)	mapNode[list.at(i).nodeName().toUpper().at(0).toLatin1()]=list.at(i);

	m_server = UA_Server_new();
	m_config =UA_Server_getConfig(m_server);
//	UA_ServerConfig_setDefault(m_config);
	UA_ServerConfig_setMinimal(m_config,ser->lport, nullptr);
	m_config->maxSessions=sr->maxNumC;

	int szUsers=ser->UserPasswd.size();
	if(szUsers>0){
		UA_UsernamePasswordLogin logins[szUsers];
		for(int i=0;i<szUsers;i++){
			logins[i].username=UA_String_fromChars(ser->UserPasswd.at(i).login.toUtf8().constData());
			logins[i].password=UA_String_fromChars(ser->UserPasswd.at(i).passwd.toUtf8().constData());
		};
	   m_config->accessControl.deleteMembers(&m_config->accessControl);
/*	   UA_AccessControl_default(UA_ServerConfig *config, UA_Boolean allowAnonymous,
	                            const UA_ByteString *userTokenPolicyUri,
	                            size_t usernamePasswordLoginSize,
	                            const UA_UsernamePasswordLogin *usernamePasswordLogin) */
	   const UA_String noneUri=UA_STRING("http://opcfoundation.org/UA/SecurityPolicy#None");
	   UA_AccessControl_default(m_config,false,&noneUri,szUsers, logins);
	   for(int i=0;i<szUsers;i++){
		   UA_String_deleteMembers(&logins[i].username);
		   UA_String_deleteMembers(&logins[i].password);
		};
	}




	UA_StatusCode s = UA_Server_run_startup(m_server);
	if (s != UA_STATUSCODE_GOOD){   qFatal("could not launch OPSUA server"); return;}
	else                           qDebug("launch OPCUA server");

	rootObject=addFolder(UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),300,"Data","The data");
	mMap::iterator it=map->begin();


	  //делаем эмуляцию
	//-------------------------------------------------
	for(;it!=map->end();it++){
		int modbus=it.key();
		UA_NodeId curNodeMdb=addFolder(rootObject,modbus,"Device_"+QString::number(modbus), "Device at "+QString::number(modbus));
		dStat ds(addVariable(curNodeMdb,300+modbus,false,QOpcUa::Types::Boolean,"Status"),false);
		actData.insert(modbus,ds);
		emulNode(modbus,true);
	   UA_NodeId_deleteMembers(&curNodeMdb);
	}
	//--------------------------------------


	m_timer=new QTimer(this);
	m_timer->setSingleShot(false);
	m_timer->setInterval(0);
	connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));//в том-же потоке


	d_timer=new QTimer(this);
	d_timer->setSingleShot(false);
	d_timer->setInterval(1000);
	connect(d_timer, SIGNAL(timeout()), this, SLOT(updateDat()));

	m_timer->start();
	d_timer->start();
}



UA::~UA(){
	UA_NodeId_deleteMembers(&rootObject);//не обязательно - строк нет
	UA_Server_delete(m_server);
	if(userPass) delete(userPass);
}

void UA::stop(){
	d_timer->stop();
	UA_StatusCode s=UA_Server_run_shutdown(m_server);
	if (s == UA_STATUSCODE_GOOD) qDebug("stop OPCUA server");
	m_timer->stop();
	emit isStoped();
}

void UA::update(){
	UA_Server_run_iterate(m_server, true);
}


quint32 UA::genNumEn(quint32 m,quint8 n,const uchar t){
	    quint32 rez=0;
        switch(t){
        	case T:
        	case N:
        		rez+=n*100*1000;
        		break;
        	case A:
        	case C:
        	case D:
        	case M:
        	case I:
        		rez+=n*10*1000;
        		break;
        	case E:
        	case F:
        		rez+=n*1000;
        		break;
        }
	    rez+=m;
	    return rez;
}

uchar UA::strToInf(const QString& txt){
	uchar t=0;
	switch(txt.at(0).toLatin1()){
	  	  case 'A': t=A; break;
	  	  case 'C': t=C; break;
	  	  case 'D': t=D; break;
	  	  case 'M': t=M; break;
	  	  case 'I': t=I; break;
	  	  case 'T': t=T; break;
	  	  case 'N': t=N; break;
	  	  case 'E': t=E; break;
	  	  case 'F': t=F; break;
	  }
	return t;
}

QString UA::infToString(uchar t){
	QString txt;
	switch(t){
	  	  case A: txt="A"; break;
	  	  case C: txt="C"; break;
	  	  case D: txt="D"; break;
	  	  case M: txt="M"; break;
	  	  case I: txt="I"; break;
	  	  case T: txt="T"; break;
	  	  case N: txt="N"; break;
	  	  case E: txt="E"; break;
	  	  case F: txt="F"; break;
	  }
	return txt;
}



void UA::updateStat(unsigned char m){
	   mMap::iterator it=map->find(m);
	   if(it!=map->end()){
	   bool stat=it.value()->dstatus;
		   QHash<unsigned short,dStat>::iterator ita=actData.find(m);
		   if(ita!=actData.end()){
				if(stat!=ita.value().stat){
					setVariableBool(ita.value().opcNode,stat);
					if(memDat.contains(m)){
						mdCan*  curL=memDat[m];
						mdCan::iterator its=curL->begin();
						if(stat==false){
							for(;its!=curL->end();its++){
								//статус не умеет возвращать норму
								setStatusCodeNode(its.value().opcNode,UA_STATUSCODE_UNCERTAINNOCOMMUNICATIONLASTUSABLEVALUE);
							}
						}else{//делаем так, что значение поменялось - дабы обновить
							for(;its!=curL->end();its++){
								its.value().value=its.value().value+1;
							}
						}
					}
					ita.value().stat=stat;
				}
		   }
	   }
}


void UA::updateDat(){
	bool stat;
	uchar k;
	mMap::iterator it=map->begin();
	for(;it!=map->end();it++){
        stat=it.value()->dstatus;
        k=it.key();
        updateStat(k);
		if(stat) testData(k);

	}
}

void UA::testData(unsigned char m){

	if(memDat.contains(m) && map->contains(m) ){

		MArray* mdC=map->value(m);
		mdCan*  curL=memDat[m];
		short cd;
		mdCan::iterator it=curL->begin();
		for(;it!=curL->end();it++){
			cd=mdC->getMData(it.key());
			if(cd!=it.value().value){
				it.value().value=cd;
			    float cf;
			    if(it.value().div!=0) cf=((float)cd)/it.value().div;
			    else                  cf=(float)cd;
			    setVariableFloat(it.value().opcNode,cf);
			   // UA_StatusCode rez=
		  	   // qDebug()<<it.key()<<it.value().value<<cd<<it.value().opcNode.identifier.numeric<<rez;
			}
		}
	}
}

void UA::deleteNodes(int modbus,uchar t,int i){
	 int ad=genNumEn(modbus,i+1,t);

	 // это временно - удаляем папки потомков
	 UA_Server_deleteNode(m_server,UA_NODEID_NUMERIC(ns,ad+1000000),false);
	 UA_Server_deleteNode(m_server,UA_NODEID_NUMERIC(ns,ad+2000000),false);
	 //---------------------------------------------------------
	 //теперь родитель
	 UA_Server_deleteNode(m_server,UA_NODEID_NUMERIC(ns,ad),false);


}

void UA::addNodesEmul(int modbus,uchar t,int i){

	  QDomElement curE;
	  QDomNodeList dList;
	  ushort sm,id;
	  bool ok;
	  float val=0;
	  QString ts=infToString(t);
	  if(!mapNode.contains(ts.at(0).toLatin1())){
		  qDebug()<<QString("Error! Not '%1' to modbus file.").arg(ts);
		  return;
	  }

	  curE=mapNode[ts.at(0).toLatin1()].toElement();
	  dList= curE.childNodes();

	  UA_NodeId curNodeMd= UA_NODEID_NUMERIC(ns,modbus);


	  int ad=genNumEn(modbus,i+1,t);
	  UA_NodeId curNodeKan=addFolder(curNodeMd,ad,ts+QString::number(i+1));
	  UA_NodeId curNodeDinamic=addFolder(curNodeKan,ad+1000000,"Dynamic","Dynamic data");
	  UA_NodeId curNodeStatic= addFolder(curNodeKan,ad+2000000,"Static", "Static data");
	  UA_NodeId curNode;

	  sm=curE.attribute("s"+QString::number(i+1),"0").toUShort(&ok,16);
	  if(sm==0) return;
	  QList<ushort> readSR=dataServer::stringRegToUShort(dataServer::genStatReg(sm,t));

	  for(int j=0;j<dList.size();j++){
			  if(dList.at(j).attributes().namedItem("read").nodeValue()!="0"){
				 id=dList.at(j).attributes().namedItem("id").nodeValue().toUShort(&ok,16);
				if(readSR.contains(sm+id)) 	curNode=curNodeStatic;
				else 						curNode=curNodeDinamic;
				int sid=(sm+id)*1000+modbus;
				emulList[modbus].append(sid);
				UA_NodeId node=addVariable(curNode,sid,
						 static_cast<float>(val),QOpcUa::Types::Float,
						 dList.at(j).attributes().namedItem("name").nodeValue(),
						 dList.at(j).attributes().namedItem("title").nodeValue());
				setStatusCodeNode(node,UA_STATUSCODE_UNCERTAININITIALVALUE);

			  }
		}
}

void UA::genData(int m){
	genDataList(m);
}

void UA::addCan(mdCan* curCan,int modbus,uchar t,int i,const QList<ushort>& readDR,const QList<ushort>& readSR){

	mdCan  staticCan;
	MArray*  mdbD=map->value(modbus);
	UA_NodeId curNodeMd= UA_NODEID_NUMERIC(ns,modbus);
	QString txi=infToString(t);
	QDomElement curE=mapNode[txi.toLatin1().at(0)].toElement();
	QDomNodeList dList= curE.childNodes();
	bool ok;

	int ad=genNumEn(modbus,i+1,t);
	UA_NodeId curNodeKan=addFolder(curNodeMd,ad,txi+QString::number(i+1));
	UA_NodeId curNodeDinamic=addFolder(curNodeKan,ad+1000000,"Dynamic","Dynamic data");
	UA_NodeId curNodeStatic= addFolder(curNodeKan,ad+2000000,"Static", "Static data");
	UA_NodeId curNode;
	ushort sm=curE.attribute("s"+QString::number(i+1)).toUShort(&ok,16);
	for(int j=0;j<dList.size();j++){
	  if(dList.at(j).attributes().namedItem("read").nodeValue()!="0"){
		 ushort id=dList.at(j).attributes().namedItem("id").nodeValue().toUShort(&ok,16);
		 mdCan::iterator it;
		 if(readDR.contains(sm+id)){
			 it= curCan->insert(sm+id,mdbD->getMData(sm+id));
			 curNode=curNodeDinamic;
		 }else if(readSR.contains(sm+id)){
			 it= staticCan.insert(sm+id,mdbD->getMData(sm+id));
			 curNode=curNodeStatic;
		 }else continue;
		 float value;
		 it.value().num=i+1;
		 it.value().kType=txi.toLatin1().at(0);
		 it.value().id=id;
		 it.value().div=dList.at(j).attributes().namedItem("divider").nodeValue().toUShort(&ok,10);
		 if(it.value().div>1) value=(float)it.value().value/it.value().div;
		 else                 value=(float)it.value().value;
		 it.value().opcNode=addVariable(curNode,(sm+id)*1000+modbus,
				 static_cast<float>(value),QOpcUa::Types::Float,
				 dList.at(j).attributes().namedItem("name").nodeValue(),
				 dList.at(j).attributes().namedItem("title").nodeValue());

	  }
   }
}


void UA::emulNode(int modbus,bool add){

	 //это временно всыязи с глюком в библиотеке - удаляем все эмулируемые ранее параметры
	if(add==false){
		 for(int i=0; i< emulList[modbus].size();i++){
				 UA_Server_deleteNode(m_server,UA_NODEID_NUMERIC(ns,emulList[modbus].at(i)),false);
		 }
		 emulList[modbus].clear();
	}
    //-----------------------------------------------------------

   QStringList list=map->value(modbus)->emulation.split('|');
   for(int i=0;i<list.size();i++){
	   QString str=list.at(i);
	   for(int j=0;j<str.size();j++){
			 if(str.at(j).toLatin1()!='0'){
				if(add) addNodesEmul(modbus,strToInf(str.mid(j,1)),j); //добавляем каналы
				else    deleteNodes(modbus,strToInf(str.mid(j,1)),j); //Удаляем каналы
			 }
	   }
   }
}

void UA::genDataList(int modbus){
	if(memDat.contains(modbus)){//не первый раз - обновить и на выход
		updateStat(modbus);
		testData(modbus);
		return;
	}

	mdCan* curCan;

//тут надо удалить эмуляции каналов
	emulNode(modbus,false);
//--------------------------------------

	UA_NodeId curNodeMd= UA_NODEID_NUMERIC(ns,modbus);


	infoADC& inf=map->value(modbus)->inf;
	//список всех читаемых по каналу динамических регистров
	QList<ushort> readDR=dataServer::stringRegToUShort(dataServer::genDinReg(&inf));
	//список всех читаемых по каналу статических регистров
	QList<ushort> readSR=dataServer::stringRegToUShort(dataServer::genStatReg(&inf));

	memDat[modbus] = new mdCan();
	curCan=memDat[modbus];


	bool statOK=true;
	QHash<ushort,dStat>::iterator ita=actData.find(modbus);
	setVariableBool(ita.value().opcNode,statOK);
	ita.value().stat=statOK;

	//тахометры

   for(int i=0; i<inf.tah.size();i++){
		if(inf.tah[i])	addCan(curCan,modbus,inf.tah[i],i,readDR,readSR);
	}
	//основные
	for(int i=0; i<inf.adc.size();i++){
	   if(inf.adc[i]) addCan(curCan,modbus,inf.adc[i],i,readDR,readSR);
	}
	//виртуальные каналы
	for(int i=0; i<inf.vir.size();i++){
	   if(inf.vir[i]) addCan(curCan,modbus,inf.vir[i],i,readDR,readSR);
 	}
	//термометры
   for(int i=0; i<inf.term.size();i++){
		if(inf.term[i]) addCan(curCan,modbus,inf.term[i],i,readDR,readSR);
	}

}


UA_NodeId UA::addFolder(const UA_NodeId &folder, UA_UInt32 id, const QString &name,	const QString &description){

	UA_NodeId resultNode;

	UA_ObjectAttributes oAttr = UA_ObjectAttributes_default;
    oAttr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", name.toUtf8().constData());
    if (description.size())  oAttr.description = UA_LOCALIZEDTEXT_ALLOC("en-US", description.toUtf8().constData());
	else                     oAttr.description = UA_LOCALIZEDTEXT_ALLOC("en-US", name.toUtf8().constData());

    UA_StatusCode result = UA_Server_addObjectNode(m_server, UA_NODEID_NUMERIC(ns, id),
							folder,
							UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
							UA_QUALIFIEDNAME(ns, name.toUtf8().data()),
							UA_NODEID_NUMERIC(0, UA_NS0ID_FOLDERTYPE),
							oAttr, NULL, &resultNode);
	UA_ObjectAttributes_deleteMembers(&oAttr);

	if (result != UA_STATUSCODE_GOOD) {
		qWarning() << "Could not add folder:" << name << " :" << result;
		return UA_NODEID_NULL;
	}
	return resultNode;
}


UA_NodeId UA::addVariable(const UA_NodeId &folder, UA_UInt32 id,
		            const QVariant &value,QOpcUa::Types type,
				   const QString &name,const QString &description){

	UA_NodeId resultNode;

    UA_VariableAttributes attr = UA_VariableAttributes_default;
    attr.value =       QOpen62541ValueConverter::toOpen62541Variant(value, type);
    attr.displayName = UA_LOCALIZEDTEXT_ALLOC("en-US", name.toUtf8().constData());
    attr.dataType =    attr.value.type ? attr.value.type->typeId : UA_TYPES[UA_TYPES_BOOLEAN].typeId;
    attr.accessLevel = UA_ACCESSLEVELMASK_READ; // | UA_ACCESSLEVELMASK_WRITE;
    if (description.size())  attr.description = UA_LOCALIZEDTEXT_ALLOC("en-US", description.toUtf8().constData());
    else                     attr.description=  UA_LOCALIZEDTEXT_ALLOC("en-US", name.toUtf8().constData());

    UA_StatusCode result = UA_Server_addVariableNode(m_server,
    							UA_NODEID_NUMERIC(ns,id),
    							folder,
    							UA_NODEID_NUMERIC(0, UA_NS0ID_HASCOMPONENT),
    							UA_QUALIFIEDNAME(ns, name.toUtf8().data()),
    							UA_NODEID_NUMERIC(0, UA_NS0ID_BASEDATAVARIABLETYPE),
								attr, NULL, &resultNode);

    UA_VariableAttributes_deleteMembers(&attr);

    	if (result != UA_STATUSCODE_GOOD) {
    		qWarning() << "Could not add variable:" << name << " :" << result;
    		return UA_NODEID_NULL;
    	}
    	return resultNode;

}






//******************************************************************


opcuaServer::opcuaServer(mMap* m, QDomDocument doc,QObject *parent) : QObject(parent) {
	// TODO Auto-generated constructor stub
	map=m;
	work=false;
	xmldoc=doc;

}

opcuaServer::~opcuaServer() {
	stop();
}

void opcuaServer::start(QDomElement sElem){

	if(sElem.isNull())             return;
	if(sElem.nodeName()!="opc_ua") return;  //!!!!!!!
	if(xmldoc.isNull())            return;

	ser=sElem;

	UserPasswd.clear();
	bool ok;
	QString port, maxNumConn;
	port=ser.attribute("port","43344");
	maxNumConn=ser.attribute("maxNumConn","10");
	lport=port.toInt(&ok,10);
	maxNumC=maxNumConn.toInt(&ok,10);



	QDomNodeList nl=ser.childNodes();
	QStringList listL;
	for(int i=0;i<nl.size();i++){
		QString login,passwd;
		login=nl.at(i).toElement().attribute("login","");
		passwd=nl.at(i).toElement().attribute("password","");
		if(login!="" && passwd!=""){//диапазон
			if(!listL.contains(login)){
				UserPasswd.append(LP(login,passwd));
				listL.append(login);
			}
		}
	}



	if(!ThredRec){
		ThredRec=new opcThread(map,this);
		connect(this, SIGNAL(stopAll()),  ThredRec, SIGNAL(stop()));
		ThredRec->moveToThread(ThredRec);
		ThredRec->start();
		work =true;
	}
}


void opcuaServer::stop(){

	if(work){
		emit stopAll();
		if(ThredRec){
			ThredRec->wait();
			delete(ThredRec);
			ThredRec=NULL;
		}
		work=false;
	}
}

UA_StatusCode UA::setVariableFloat(UA_NodeId& id, float val,UA_StatusCode st){
	    UA_WriteValue wv;
	    UA_Variant myVar;
	    UA_Variant_init(&myVar);
	    UA_Variant_setScalar(&myVar, &val, &UA_TYPES[UA_TYPES_FLOAT]);
	    UA_WriteValue_init(&wv);
	    wv.nodeId = id;
	    wv.attributeId = UA_ATTRIBUTEID_VALUE;
	    wv.value.status = st;
	    wv.value.value = myVar;
	    wv.value.hasStatus = true;
	    wv.value.hasValue = true;
	    return UA_Server_write(m_server, &wv);
}
UA_StatusCode UA::setVariableBool(UA_NodeId&  id, bool  val,UA_StatusCode st){
	 UA_WriteValue wv;
	 UA_Variant myVar;
	 UA_Variant_init(&myVar);
	 UA_Variant_setScalar(&myVar, &val, &UA_TYPES[UA_TYPES_BOOLEAN]);
	// UA_Variant_setScalarCopy(&myVar, &val, &UA_TYPES[UA_TYPES_BOOLEAN]);
	 UA_WriteValue_init(&wv);
	 wv.nodeId = id;
	 wv.attributeId = UA_ATTRIBUTEID_VALUE;
	 wv.value.status = st;
	 wv.value.value = myVar;
	 wv.value.hasStatus = true;
	 wv.value.hasValue = true;
	 return UA_Server_write(m_server, &wv);
}

UA_StatusCode UA::setVariableInt(UA_NodeId&  id, int  val,UA_StatusCode st){
	 UA_WriteValue wv;
	 UA_Variant myVar;
	 UA_Variant_init(&myVar);
	 UA_Variant_setScalar(&myVar, &val, &UA_TYPES[UA_TYPES_INT32]);
	// UA_Variant_setScalarCopy(&myVar, &val, &UA_TYPES[UA_TYPES_INT32]);
	 UA_WriteValue_init(&wv);
	 wv.nodeId = id;
	 wv.attributeId = UA_ATTRIBUTEID_VALUE;
	 wv.value.status = st;
	 wv.value.value = myVar;
	 wv.value.hasStatus = true;
	 wv.value.hasValue = true;
	 return UA_Server_write(m_server, &wv);
}

//на нормальный статус не переключает!!!
void UA::setStatusCodeNode(UA_NodeId& id,UA_StatusCode st){

	UA_WriteValue wv;
	UA_Variant myVar;
	UA_Server_readValue(m_server,id,&myVar);
	UA_WriteValue_init(&wv);

	wv.nodeId = id;
	wv.attributeId = UA_ATTRIBUTEID_VALUE;
	wv.value.status = st;
	wv.value.hasStatus = true;
	wv.value.value = myVar;
	wv.value.hasValue = true;
	UA_Server_write(m_server, &wv);
}

