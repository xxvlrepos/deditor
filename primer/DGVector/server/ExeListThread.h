/*
 * ExeListThread1.h
 *
 * Поток опроса для одной или нескольких шарманок
 * Соединение осуществляется через первую
 *
 *  Created on: 02 окт. 2014 г.
 *      Author: Usach
 */

#ifndef EXELISTTHREAD_H_
#define EXELISTTHREAD_H_

#include <QtCore>
#include <QObject>
#include <execut.h>
#include <QThread>
#include "mMap.h"
#include <dgserver_global.h>

//class dataServer;

//*****************************************************************************
/*

 */

class DGSERVER_EXPORT ExeListThread: public QThread
{
     Q_OBJECT

public:

    QList<execut*> lexe;
    static uchar  debugN;

    ExeListThread(const QList<tape*>& ltp,mMap*m, QObject *parent=0);
    ~ExeListThread(){
    	haltALL();
    	//qDebug()<<"destruct ExeListThread";
     }




    void restartExe(int n,tape* t);
    void restartExe(int n);
    void run();

 public slots:
    void stop();

  signals:
   void   finished(int);
   void   restartN(int);
   void   finStat(int);
   void   finDin(int);
   void   update(QByteArray,QByteArray);
   void   message(QString,int=0);


private:
	QList<tape*> lttp;
	mMap* map;
	QSignalMapper* signalMapper;
	QHash<unsigned char,bool> endDin;
	QHash<uchar,int>  statH;
	int maxS;
	bool runS;
	signals:
	  void halt();


private slots:
    void finish(int n);
	void startN(int n);
	void haltALL();
	void headRez(command*);
	void mError(QString t) {emit message(t,1);}
	void statCon(uchar,int,int);
};


#endif /* EXELISTTHREAD_H_ */
