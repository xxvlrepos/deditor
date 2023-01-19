/*
 * xmlConverter.cpp
 *
 *  Created on: 11 июля 2014 г.
 *      Author: Usach
 */

#include "xmlConverter.h"
#include <QXmlSchema>
#include <QXmlSchemaValidator>

xmlConverter::xmlConverter(deditor* edit) {
	// TODO Auto-generated constructor stub
	editor=qobject_cast<deditor*>(edit);  //получаем указатель на родителя как на обьект типа deditor
	if(!editor) ini();
	else{
		xmlNameD=editor->getNameXml();
		xsdNameD=editor->getNameXls();
		rootNodeD=editor->rootNode;

	}

}

xmlConverter::~xmlConverter() {
	// TODO Auto-generated destructor stub
}

bool xmlConverter::ini(){
	 xmlNameD=XSD::createFile(XSD::appPath+"/"+XSD::SettingsXML);
	 if(!XSD::xmlValidator(XSD::SettingsXSD,xmlNameD,&xmldocD)) return false;
	 xsdNameD=XSD::SettingsXSD;
	 rootNodeD=xmldocD.documentElement();

	 return true;
}


bool xmlConverter::save(){

	if(xmldocD.isNull()) return false;
	if(xmlNameD=="")     return false;
	QFile file(xmlNameD);
	if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
		qDebug() <<"Not open write file:"+xmlNameD;
		return false;
	}
	QTextStream out(&file);
	out.setCodec("UTF-8");
	out<<xmldocD.toString();
	file.close();
	return true;
}




bool xmlConverter::convert(const QString& NameFD,const QString& xsdName){

	if(rootNodeD.isNull()) return false;
	if(!XSD::xmlValidator(xsdName,NameFD,&xmldocS)) return false;
	xmlNameS=NameFD;
	return convert(xmldocS);
}

bool xmlConverter::convert(const QDomDocument& doc){

	xmldocS=doc;
	rootNodeS=xmldocS.documentElement();


	QDomNodeList cl=rootNodeS.childNodes();
	QDomElement el;
	bool ok;
	unsigned char imdb;
	map.clear();

	for(int i=0;i<cl.size();i++){
		el=cl.at(i).toElement();
		imdb=el.attribute("modbus").toUShort(&ok,10);
		if(!map.contains(imdb)){
			dat ar;
			ar.id=el.attribute("id","");
			ar.ip=el.attribute("ip","");
			ar.port=el.attribute("port","");
			ar.alias=el.attribute("alias","");
			ar.bitrate=el.attribute("bitrate","");
			ar.setVer(el.attribute("version",""));
			ar.setModbus(el.attribute("modbus","0"));
			ar.setBitrate(el.attribute("bitrate","0"));
			ar.setMaster(el.attribute("master","0"));

			if(ar.dver<ar.vdat){
				qDebug()<<ar.id+" older firmware - is disable";
			}
			else{
				map.insert(imdb,ar);
				qDebug()<<ar.id+" to modbus "+ar.modbus+ " added";

			}
		}else  qDebug()<<el.attribute("id","")+" doubling addres - is disable";
	}

	//Разбираемся с типами соединений
	mUDP.clear();
	mTCP.clear();
	mCOM.clear();
	QHash<unsigned char,dat>::iterator it=map.begin();
	for(;it!=map.end();it++){
		if((*it).dver<(*it).vdat) continue;
		if((*it).ip!="")  mTCP[(*it).ip].append((*it).imodbus);
		else mCOM[(*it).port].append((*it).imodbus);
	}
	//Одиночные переносим из mTCP в mUDP
	QList<QString> key=mTCP.keys();
	for(int i=0;i<key.size();i++){
		if(mTCP[key[i]].size()==1){
			mUDP[key[i]].append(mTCP[key[i]].first());
			mTCP.remove(key[i]);
		}
	}

	//формируем файл настроек шлюза
	QDomElement clients=rootNodeD.firstChildElement("client");
	if(clients.isNull()) return false;

	if(editor){//удаляем всех клиентов
		editor->clearNode(editor->indexTreeFromNode(clients));
	}else{
		while(!clients.firstChild().isNull()) clients.removeChild(clients.firstChild());
	}

	QString txt;
	unsigned char m;
	QDomElement ne,nc;
	key=mUDP.keys();
	for(int i=0;i<key.size();i++){
		if(editor) ne=editor->addNode(editor->indexTreeFromNode(clients),"UDP");
		else {     ne=xmldocD.createElement("UDP"); clients.appendChild(ne);  }

		m=mUDP[key[i]].first();
		ne.setAttribute("id", map[m].id );
		ne.setAttribute("ipaddr",key[i]);
		ne.setAttribute("port","502");
		ne.setAttribute("modbus",txt.setNum(m));
		ne.setAttribute("alias",map[m].id);
	}


	key=mTCP.keys();
	QStringList TcpL;
	for(int i=0;i<key.size();i++){

		if(!TcpL.contains(key[i])){
			if(editor) ne=editor->addNode(editor->indexTreeFromNode(clients),"TCP");
			else {     ne=xmldocD.createElement("TCP"); clients.appendChild(ne);  }

			ne.setAttribute("ipaddr",key[i]);
			ne.setAttribute("port","502");
			TcpL<<key[i];
		}

		for(int j=0;j<mTCP[key[i]].size();j++){
			if(editor) nc=editor->addNode(editor->indexTreeFromNode(ne),"modbus");
			else {     nc=xmldocD.createElement("modbus"); ne.appendChild(nc);  }
			m=mTCP[key[i]].at(j);
			nc.setAttribute("id", map[m].id );
			nc.setAttribute("modbus",txt.setNum(m));
			nc.setAttribute("alias",map[m].id );
		}
	}

	key=mCOM.keys();
	QStringList ComL;
	for(int i=0;i<key.size();i++){

		if(!ComL.contains(key[i])){
			if(editor) ne=editor->addNode(editor->indexTreeFromNode(clients),"COM485");
			else {     ne=xmldocD.createElement("COM485"); clients.appendChild(ne);  }
			ne.setAttribute("comAddr",key[i]);
			ne.setAttribute("speed",map[mCOM[key[i]].first()].bitrate);
			ComL<<key[i];
		}

		//qDebug()<<key[i]<<mCOM[key[i]].size();
		for(int j=0;j<mCOM[key[i]].size();j++){
			if(editor) nc=editor->addNode(editor->indexTreeFromNode(ne),"modbus");
			else {     nc=xmldocD.createElement("modbus"); ne.appendChild(nc);  }

			m=mCOM[key[i]].at(j);
			nc.setAttribute("id", map[m].id );
			nc.setAttribute("modbus",txt.setNum(m));
			nc.setAttribute("alias", map[m].id);
		}

	}

	bool rez=true;
	if(editor) editor->saveDom();
	else       rez=save();

	return rez;
}





