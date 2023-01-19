/*
 * const.h
 *
 *  Created on: 11 июля 2014 г.
 *      Author: Usach
 */

#ifndef CONST_H_
#define CONST_H_

#include <dgserver_global.h>
#include <QtCore>
#include <QObject>
#include <QDomDocument>


//ссылки на схемы - вынесены для удобства редактироания/отладки


namespace  XSD{
       extern const DGSERVER_EXPORT QString DevXML;
	   extern const DGSERVER_EXPORT QString ModbusXML;
	   extern const DGSERVER_EXPORT QString SettingsXML;

	   extern const DGSERVER_EXPORT QString DevXSD;
	   extern const DGSERVER_EXPORT QString ModbusXSD;
	   extern const DGSERVER_EXPORT QString SettingsXSD;

	   extern   DGSERVER_EXPORT     QString appPath;
		
       DGSERVER_EXPORT bool  xmlValidator(const QString& xsdName, const QString& Name,QDomDocument* rez);
       DGSERVER_EXPORT QString  createFile(QString Name,QString srcDir=".",bool* crDef=0);
}

namespace RPC{
    extern const DGSERVER_EXPORT quint16 Port;
}
namespace SYSTEM{
    extern DGSERVER_EXPORT bool Debug;
    extern DGSERVER_EXPORT uchar   NumDebug;
    extern DGSERVER_EXPORT QString LogFileName;
}


#endif /* CONST_H_ */
