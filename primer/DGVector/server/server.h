/*
 * server.h
 *
 *  Created on: 04 дек. 2014 г.
 *      Author: Usach
 */



#ifndef SERVER_H_
#define SERVER_H_


#include <const.h>
#include <QDomDocument>
#include <QDomNode>
#include <QThread>
#include <mMap.h>

#include <dataServer.h>
#include <mdbServer.h>
#include <oscCollector.h>
#include <sdiagCollector.h>
#include <dbOSCFile.h>
#include <dataCollector.h>
#include <maiaXmlRpcServer.h>
#include <logFile.h>
#include <opcuaServer.h>



extern DGSERVER_EXPORT mMap MDBArray;         //Собственно - главный массив данных со своими методами

/* mMap - словарь типа QHash<unsigned char,MArray*>, где unsigned char - модбас адрес, MArray - хранилище данных блока
  Функции для работы см. в описании  QHash

  Для получения значений из хранилища MArray используются функции
   short getMData(int ma);                  //получить одно значение по адресу регистра
   QVector<short> getMData(int ma,int len); //значения по адресу первого регистра и количеству регистров
   Пример  short a=MDBArray[1]->getMData(0x1001);

 дополнительные интерфейсные функции mMap
  QMap<unsigned char,int>  getStatus(); //получить список текущих состояний приборов ( eStat {eStop=0,eInfo,eData})
  QMap<unsigned char,bool> getDStatus();//получить список - данные прибора получаем или нет
*/

//******************************************************
/*Список экспортируемых классов
class DGSERVER_EXPORT  server;    //сервер для запуска во внешней очереди сообщений
class DGSERVER_EXPORT  execut;    //шарманка для запуска опросов блоков
class DGSERVER_EXPORT  mdata;     //элемент - описание параметров прибора
 */
 //*********************************************************************

/* Внимание!!!!!!
Перед запуском надо задать значение глобальной переменной - полный путь к папке программы!!!!
extern DGSERVER_EXPORT    QString XSD::appPath;

Еще есть служебный класс class DGSERVER_EXPORT sFile : public QFile
Это файл лога! если создать глобальный обьект данного класса   sFile Log(LogFileName);
то все сообщения будут попадать в него.
*/
//*********************************************************************************************
//даннык класс запускается только во внешней очереди сообщений, автономный - см ниже (tserver)



//class DGSERVER_EXPORT server;

class DGSERVER_EXPORT server: public QObject{

	Q_OBJECT

public:

    static QString ver;
    static QString nsvn;
	static server* MyServer;
	server(mMap* m=&MDBArray,QObject *parent = 0);
	virtual ~server();


	//состояния
public slots:

    bool  start(const QString& Name=XSD::appPath+"/"+XSD::SettingsXML,
    		    const QString& xsdName=XSD::SettingsXSD);
    void  stop();
//    void dstart(int delay=0,int cou=0);//старт с временной задержкой
	void restart(QString nm="");

	void startRPC(){QMetaObject::invokeMethod(this,"start",Qt::QueuedConnection);}
	void stopRPC() {QMetaObject::invokeMethod(this,"stop", Qt::QueuedConnection);}
	bool isRun(){return run;}            //состояние вообще
	QVector<int> getStatusOSCCol();       //сбор осцилограмм - задания
	QVariantList getStatusOSCColV();
	QStringList  getLogTxt(QDateTime);
	QByteArray   getSettings();
	QByteArray   getModbusSett();
	bool         setSettings(QByteArray);
	bool         setModbusSett(QByteArray);

	QMap<unsigned char,int> getStatus(){return map->getStatus();};  //статус приборов
	QVariantMap             getStatusV();
	QString                 getVer(){return ver;}
	int getStatusDB();                    //коннект с базой




private:


		bool run;

		mMap* map;
		dataServer*    dServer;
		mdbServer*     mServer;
		oscCollector*  oscColl;
		sdiagCollector* sdiagColl;
		dbOSCFile*     dbOSC;
		dataCollector* dbColl;
		MaiaXmlRpcServer* xmlRPC;
		opcuaServer*      uaServer;
		int couDBCon;
        int conDelay;
        int conCou;

		QString xmlName,xsdName;
	//	QString xmlMName,xsdMName;
		QDomDocument xmldoc;
		QDomElement rootNode;//
		QDomDocument mdbXmlDoc;//описание регистров модбас
		QString dbType;
	//	bool testDBService(QDomElement,QString&);
		bool fileSave(const QString&,const QDomDocument&);
		void testFile(QString Name,QString srcDir,QString xsdName);

};



#endif /* SERVER_H_ */
