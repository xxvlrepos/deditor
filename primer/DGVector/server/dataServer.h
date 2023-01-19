/*
 * dataServer.h
 *
 *  Сервер опроса блоков.
 *  Является головным классом для всех остальных.
 *
 *  Created on: 06 июня 2014 г.
 *      Author: Usach
 */

#ifndef DATASERVER_H_
#define DATASERVER_H_


#include <QtCore>
#include <QObject>
#include <QDomDocument>
#include <QDomNode>
#include <execut.h>
#include <QThread>
#include "mMap.h"
#include "ExeListThread.h"
#include "snifferThread.h"

enum typeDev{
	T=0x10,
	A=0xA0,
	C=0xC0,
	D=0xD0,
	E=0xE0,
	F=0xF0,
	M=0x20,
	I=0x30,
	N=0x40
};


class UpdateThread: public QThread{
	Q_OBJECT

public:
	QList<QThread* > *list;

	UpdateThread(QList<QThread* > *l,QObject *parent=0): QThread(parent){
		list=l;
	}
	~UpdateThread(){stop();}

    void run(){
    	for(int i=0;i<list->size(); i++){
    		if(ExeListThread* threadd =qobject_cast<ExeListThread *>(list->at(i))){
				connect(threadd,  SIGNAL(update(QByteArray,QByteArray)),
					this, SLOT(update(QByteArray,QByteArray)),
					Qt::QueuedConnection);

				threadd->start(TimeCriticalPriority);
    		}

    	}
    	exec();

    }

public slots:
        void stop(){
        	//qDebug()<<"treadList:"<<list->size();
        	while(list->size()>0){
        		if(ExeListThread* threadd =qobject_cast<ExeListThread*>(list->takeLast())){
					disconnect(threadd,  SIGNAL(update(QByteArray,QByteArray)),
							   this, SLOT(update(QByteArray,QByteArray)));
					threadd->stop();
					threadd->wait();
					delete(threadd);
					continue;
        		}


			}
        	quit();
        }

public slots:
   void update(QByteArray sr, QByteArray rs){
	   mMap::mdbMap->updateData(sr,rs);
   }
};

//******************************************************************************

class dataServer: public QObject{
	Q_OBJECT
public:


	static const QThread::Priority pReadD=QThread::NormalPriority;
    static const QVector<unsigned short> mdbsADC,mdbsTAH,mdbsE,mdbsF,mdbsTERM; //смещения регистров
    DGSERVER_EXPORT static uchar NumDebugDev;
    DGSERVER_EXPORT static uchar NumDebugCycle;

	dataServer(mMap*,QObject *parent = 0);
	virtual ~dataServer();


	bool start(QDomElement,QDomDocument=QDomDocument(),bool fullMirr=true);
	void stop();

    int getTimeOut(){return TimeOut;};


	//Параметры - отправленная и полученная модбас посылка
	void updateData(const QByteArray&,const QByteArray&);

	static QStringList genDinReg(infoADC*);
	static QStringList genDinReg(infoADC* inf,QDomDocument mdoc);
	static QStringList genStatReg(infoADC*);
	static QStringList genStatReg(ushort ba, uchar t);
	static QString genSTRA(ushort a,ushort l);
	static QString genSTRL(ushort sm,ushort a1,ushort l);
	static QString genSTRD(ushort sm,ushort a1,ushort a2);
	static QStringList genInfReg();
	static QList<ushort> stringRegToUShort(const QStringList& ls);

	signals:
	   void finStat(int);
	   void finDin(int);
	//   void stoped();


protected:

	struct datP{
			QString parCon;
			QString proto;
			uint samplingIdle;
			uint samplingRate;
			uint timeOut;
			uint repeat;
			bool fullMirror;
	  		QList<unsigned char> mlist;
	  		datP(){
	  			samplingIdle=5000;
	  			samplingRate=500;
	  			timeOut=100;
	  			repeat=0;
	  			fullMirror=true;
	  		}
	};
	typedef QMap<QString,datP> conMap;
	bool fullMirror;


	mMap* map;
	UpdateThread* uThread;

	int Repeat;  //число повторов при потере
	int SamplingIdle; //период повторных опросов при разрыве
	int SamplingRate; //период опроса
	int TimeOut;      //ждем ответ

	conMap mUDP, mTCP, mCOM;

	QList<QByteArray> fileReadArr(QStringList str);//генерит строку чтения файла из списка регистров модбас и длинн

	/*потоки групповые */

	QList<tape*> addUDP_LT(QString ip, QString port, unsigned char m,datP);
	QList<tape*> addTCP_LT(QString ip, datP);
	QList<tape*> addCOM_LT(QString addr,datP);
	QList<tape*> addCOM_LTV(QString addr,datP);//для опроса по протоколу вектор

	//tape* exeTCP1(QString ip,QList<unsigned char>& m);
	//tape* exeCOM1(QString com,QList<unsigned char>& m);


  signals:
	void stopAll();


protected  slots:

    void exitRun(int); //запуск второй стадии - получение данных
    void sendMess(QString s,int i=0);


private:
	union usort{
   			short sd;
   			char sc[2];
    };
	QList<QThread *> ThreadList;
	QList<snifferThread *> snifferTList;
	QDomDocument mdbDoc;

	void getMINFO(MArray*);//переводим информацию изрегистров в описание

	//modbus
	tape* exe_TIME(connectType cType,QString addr,QString par,uint samp,uint timeout,unsigned char m=0);
	tape* exe_I(unsigned char m,connectType cType,QString addr,QString par,datP p,bool setTime=true);
	tape* exe_SD(unsigned char m,connectType cType,QString addr,QString par,datP p);

	//vector
	tape* exe_OPEN_V(connectType cType,QString addr,QString par,uint samp);
	tape* exe_SV(unsigned char m,connectType cType,QString addr,QString par,datP p);
	tape* exe_DV(unsigned char m,connectType cType,QString addr,QString par,datP p);

};


#endif /* DATATASERVER_H_ */
