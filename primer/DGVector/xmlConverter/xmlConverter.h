/*
 * xmlConverter.h
 *
 *  Конвертирует файл-список приборов в настройки шлюза
 *
 *  Created on: 11 июля 2014 г.
 *      Author: Usach
 */

#ifndef XMLCONVERTER_H_
#define XMLCONVERTER_H_

#include <const.h>
#include <qdatetime.h>
#include <deditor.h>
#include <QDomDocument>
#include <QDomNode>
#include <QtCore>


struct dat{
		QString id,ip,modbus,port,bitrate,master,ver,alias;
		QDate vdat,dver;
		unsigned char imodbus;
		int  ibitrate;
		bool bmaster;
		dat(){
			vdat.setDate(2014, 05, 27);
		}
		void setModbus(QString m){bool ok; modbus=m;	imodbus=m.toUInt(&ok,10);}
		void setBitrate(QString b){bool ok; bitrate=b;	ibitrate=b.toUInt(&ok,10);}
		void setMaster(QString ma){master=ma; bmaster=(ma=="1");}
		void setVer(QString v){
			ver=v;
			QLocale loc(QLocale::English, QLocale::UnitedStates);
			dver=loc.toDateTime(ver,"yyyy-MM-dd").date();
		}
};

class xmlConverter {
public:

	xmlConverter(deditor* edit=NULL);
	virtual ~xmlConverter();

	bool convert(const QDomDocument&);
	bool convert(const QString& NameFD,const QString& xsdName=XSD::DevXSD);


	typedef QMap<QString,QList<unsigned char> > conMap;

	deditor* editor; //кем рулим
	QHash<unsigned char,dat> map;
	conMap  mUDP,mTCP,mCOM;
	QString xmlNameS,     xmlNameD;
	QString               xsdNameD;
	QDomDocument xmldocS, xmldocD;
	QDomElement rootNodeS,rootNodeD;//

protected:
	bool ini();
	bool save();

};

#endif /* XMLCONVERTER_H_ */
