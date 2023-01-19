/*
 * sdiagCollector.h
 *
 *  Created on: 28 февр. 2017 г.
 *      Author: Usach
 *
 *      Снятие диагностических спектров
 */

#ifndef SDIAGCOLLECTOR_H_
#define SDIAGCOLLECTOR_H_

#include <QtCore>
#include <QObject>
#include <QDomNode>

#include "const.h"
#include <execut.h>
#include "ExeThread.h"
#include "mMap.h"
#include "Sheduler.h"
#include "dbConnector.h"
#include "dataTableMySQL.h"
#include "dataTablePostgreSQL.h"

#include "dgserver_global.h"

class SExeTread: public ExeThread{

	Q_OBJECT

public:
	SExeTread(tape*tp, QObject *parent=0): ExeThread(tp,parent){};
	virtual void exeRez(command* cmd);

signals:
	void emitDate(QString,QByteArray);


};

class DGSERVER_EXPORT sdiagCollector;

class sdiagCollector: public QObject{

	Q_OBJECT

public:

	 struct rms{
		 rms(){cnum=0;nConv=0; start=1;size=1;frStep=0;}
		 short start;
		 short size;
		 float frStep;
		 int   nConv;  //сколько раз корректировать
		 QString stFr,enFr;
		 QString Name;
		 QString Razm;
		 QString label;
		 int     cnum;//абсолютный порядковый номер
		 QVector<int> vecBins;
	 };

	 struct Job{
		   QList<unsigned char> devList;
		   bool     hamming;
		   QString  frequency;
		   short    passes;
		   uchar     tkan;
		   QList<rms> rezList;
	 };

#ifdef _UNSHED_DIAG_
	 static const bool unsheduled=true;
#else
	 static const bool unsheduled=false;
#endif

	 static const QVector<QString> frList;
	 static const QVector<QString> nameRMS;
	 static const QVector<QString> sizeRMS;
	 static const QVector<unsigned short> bazeList;
	 static const QVector<unsigned short> bazeSpList;
	 static const int maxSizeS=2048;
	 static const int maxSizeM=125;
	 static const int startAdr=0x6000;
	 static const double TwoPi;

	 static void setDiap(QDomNode);
	 static void setAC(QDomNode);


	sdiagCollector(mMap*,QDomElement dbPar,QString& type, QObject *parent = 0);
	virtual ~sdiagCollector();
	bool start(QDomElement);

public slots:
	void stopAll();
	void iniDist(int m);


signals:
	 void stopAllThread();


protected:

   mMap*          map;
   Sheduler*      shedDiag;
   QTimer         timer;
   QDomElement    sdiag;
   QList<Job>     JList;
  // SExeTread*     eTred;



   union usort{
		short   sd;
		ushort  usd;
		char sc[2];
   };

   bool getSDiag(int& err);
   tape* exe_SDiag();


protected  slots:
   void runDiag(int);
   void runDiag(){runDiag(0);}
   void deleteThred();
   void setDate(QString,QByteArray);
   void iniTable();

private:
   bool run;  //работает поток
   bool work; //класс работает
   bool toDB;
   QDomElement parConn;
   QString cName;
   QString dbType;
   DBTable* tableDist;
   DBTable* tableSData;
   DBTable* tableName;
   dbConnector* dcon;
};


#endif /* SDIAGCOLLECTOR_H_ */
