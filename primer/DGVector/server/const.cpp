/*
 * const.cpp
 *
 *  Created on: 14 июля 2014 г.
 *      Author: Usach
 */
#include "const.h"
#include <QDomNode>
#include <QXmlSchema>
#include <QXmlSchemaValidator>

namespace XSD{

    const QString DevXML="devices.xml";
    const QString ModbusXML="modbus.xml";
    const QString SettingsXML="settings.xml";

	const QString  DevXSD=":/hml/resource/devices.xsd"; 
	const QString  ModbusXSD=":/hml/resource/modbus.xsd";
	const QString  SettingsXSD=":/hml/resource/settings.xsd";
	QString        appPath=".";


}

namespace RPC{
    const  quint16 Port=8082;
}

namespace SYSTEM{
    bool Debug=false;
    uchar  NumDebug=0;
    QString LogFileName;
}


//******************************************************************************
/*
 * Создаем файл по дефолту, если его нет
 */
QString  XSD::createFile(QString Name,QString srcDir,bool* crDef) {
    
	QString fileName;
	QString SName,PName;
	if(Name.contains('/')){
		SName=Name.section('/',-1);
		PName=Name.section('/',0,-2);
	}else{
		SName=Name;
		PName=".";
	}
	QFileInfo fileIn(PName+"/"+SName);
	if(fileIn.isFile()){
		if(fileIn.isWritable() && fileIn.size()>0){
			fileName=PName+"/"+SName;
			if(crDef) *crDef=false;
		}
	}


	if(fileName==""){

		QFile sfile(srcDir+"/"+SName);
		QFile file(PName+"/"+SName);

		if(sfile.open(QIODevice::ReadOnly | QIODevice::Text)){
			if(file.open(QIODevice::WriteOnly | QIODevice::Text)){
				fileName=PName+"/"+SName;
				QTextStream to(&file);
				to.setCodec("UTF-8");
				to<<QString::fromUtf8(sfile.readAll());
				file.close();
			}
			sfile.close();
			qWarning()<<"creade default file:"+SName;
			if(crDef) *crDef=true;
	    }
	}
	return fileName;
}

bool XSD::xmlValidator(const QString& xsdName, const QString& Name,QDomDocument* doc){

	QXmlSchema sch;
	QDomDocument xsddoc;

    QFile filexsd(xsdName);
    if(!filexsd.open(QIODevice::ReadOnly | QIODevice::Text)){
	     qDebug() <<"Not open file:"+xsdName;
	     return false;
     }else{
		if(!xsddoc.setContent(&filexsd)){
			qDebug() <<"Error parse file:"+xsdName;
			filexsd.close();
			return false;
		}else{
			filexsd.close();
			sch.load(xsddoc.toByteArray());
			if(!sch.isValid()){
				 qDebug() <<"Schema invalid:"+xsdName;
				 return false;
			}
		}
     }

    if(Name=="") return false;

	QFile file(Name);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
		qDebug() <<"Not open file:"+Name;
		return false;
	}
	if(!doc->setContent(&file)){
		file.close();
		qDebug() <<"Error parse file:"+Name;
		return false;
	}
	file.close();

	QXmlSchemaValidator validator(sch);
	if (!validator.validate(doc->toByteArray())){
		qDebug() <<"XML document invalid!";
		return false;
	}
	return true;
}

//**************************************************************************************

