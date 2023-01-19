/*
 * oscCollector.h
 *
 * Класс для сбора буферизованных осцилограмм
 * Создает непротеворечивые потоки по сбору осцилограмм с нужными параметрами
 *
 *  Created on: 02 окт. 2014 г.
 *      Author: Usach
 */

#ifndef OSCCOLLECTOR_H_
#define OSCCOLLECTOR_H_

#include <QtCore>
#include <QObject>
#include <QDomNode>

#include "const.h"
#include <execut.h>
#include "ExeThread.h"
#include "mMap.h"
#include "Sheduler.h"





class oscCollector : public QObject{

	Q_OBJECT

public:

	 struct Job{
		   Job(){id=0; MultJobClose=false;}
		   int id;
		   QList<unsigned char> devList;
		   QString resolution;
		   QString frequency;
		   bool    MultJobClose;
	 };

	static const int maxCount=262144; //максимальный размер осцилограммы 2^16
	static const QStringList frList;
	static const QStringList resList;

	oscCollector(mMap*,int timOu, QObject *parent = 0);
	virtual ~oscCollector();

	int  getOSC(Job& job, int& err,int id=-1);
	int  getStatusTread(){ return TredList.size();}
	bool start(QDomElement);
	QVector<int> getStatusShed(){ return statusList; }
	int getLifeTime(){ return lifeTime;}
	void clearOldFile(QMap<QString,QDateTime>& ,QString& ,QDateTime&,bool&);


public slots:
	void stopAll();

signals:
   void fileLoaded(QString);

protected:



   struct runJobId{
	     QList<unsigned char> devList;
  	     int id;
   };

   QString fileDir;
   int TimeOut;
   int repeatOSC;

   mMap* map;
   tape* exe_OSC(QString dir,QMap<unsigned char,QList<unsigned char> > &sp,unsigned int h,bool multi=false,unsigned char d=1,unsigned int cou=262144);
   Sheduler*      shedJob;

   bool diskFileSearch;
   QMap<QString,QDateTime> fileOSCMap;
   QFuture<void>        futureClearF;

  QHash<quint64,runJobId > runList;

  QDomElement osc;//
  int lifeTime;
  QDateTime testLT;


  QList<Job> jobList;
  QVector<int> statusList;
  QList<ExeThread*> TredList;
  void deleteThred(ExeThread* thread);


 signals:
     void stopAllThread();

 protected  slots:
    void runJob(int);
   	void deleteThred();
   	void endLoadF(QString);

private:
	bool run;
	bool work; //класс работает
	QString *fDir;
	bool testRun(unsigned char m);


};

#endif /* OSCCOLLECTOR_H_ */
