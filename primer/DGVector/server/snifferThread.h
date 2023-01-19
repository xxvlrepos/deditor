/*
 * snifferTread.h
 *
 *  Created on: 19 июня 2015 г.
 *      Author: Usach
 */

#ifndef SNIFFERTHREAD_H_
#define SNIFFERTHREAD_H_


/*
 * Поток опроса для нескольких шарманок
 */
#include <QtCore>
#include <QObject>
#include <QThread>
#include "mMap.h"
#include <QtSerialPort/QSerialPort>


namespace SYSTEM{
   extern DGSERVER_EXPORT bool Debug;
   extern DGSERVER_EXPORT uchar  NumDebug;
}

class snifferThread: public QThread
{
     Q_OBJECT

public:

	 static const int maxLen=4102*2; //максимум запрос ответ *2
	 snifferThread(QString addr,QString speed,QVector<unsigned char> ml,int del,int tdis, mMap*m, QObject *parent=0): QThread(parent){
    	 bool ok;


    	 Addr=addr;
    	 Speed=speed;
    	 mList=ml;
    	 map=m;
    	 CMD=0;
    	 lenData=0;
    	 curM=0;
    	 speedN=Speed.toUInt(&ok,10);
       	 buf.reserve(maxLen);
    	 uk=0;
    	 delay=del;
    	 if(tdis>8500) timeD=tdis;
    	 else          timeD=8500;

    }

    void run();

  signals:
       void   finStat(int);
       void   finDin(int);
       void   message(QString,int er=0);

 public slots:
    void stop();
    void Receive();

private slots:
	void breakCMDSlot();
	void testActive();

private:

    uchar CMD;
    int  lenData;
    uchar curM;
    QSerialPort* com;
    QByteArray buf;
    int        uk;

    QTimer* timer;//ждем ответа
    QTimer* activeTimer;//сечем активность

	mMap* map;
	QString Addr,Speed;
	int     speedN;
	QVector<unsigned char> mList;
	int delay;
	int timeD;

	void addData(unsigned char M,unsigned char cmd,QVector<unsigned char> data);
	char getCommand(char* cmd,int& len);
	unsigned short CRC16(char*,int len);// len- длинна в словах!!!
	void UpdateCRC16(unsigned char, unsigned short*);
	QString Array_to_String(char* arr,int len);
	void debugD(const QString& txt,uchar M,uchar C);
	void breakCMD();


	//обьединение для вычисления crc
	union ucrc{
		unsigned short	  shor;
		char			  cha[2];
		bool			  bo[16];
		signed   short	  ssh;
	};
};


#endif /* SNIFFERTHREAD_H_ */
