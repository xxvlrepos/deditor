/*
 * dbOSCFile.h
 *
 *  Created on: 31 окт. 2014 г.
 *      Author: Usach
 */

#ifndef DBOSCFILE_H_
#define DBOSCFILE_H_

#include <QtCore>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDomNode>
#include "dbConnector.h"
#include "dataTableMySQL.h"
#include "dataTablePostgreSQL.h"

class dbDataThread;



class dbOSCFile : public QObject {

	Q_OBJECT

public:
	dbOSCFile(const QString& Name,QString& type, QObject *parent);
	virtual ~dbOSCFile();
	virtual bool dbConnect(QDomElement sElem,int lt);
	bool getStatus(){return (bool)thread;};


	void iniTable();
	void FileToDB(QString fName);

	const QString& getNameConn(){return nameConn;};
	QDomElement    getParConn() {return parConn; };




public	slots:
	void addFileOSC(QString);
	void stop(){emit stopAll();};

protected:

	 int          lifeTime;
	 DBTable*     tableTOSC;

     QString      nameConn;
     QDomElement  parConn;
     dbDataThread* thread;

signals:
     void stopAll();
     void sendF(QString);

protected slots:
	void deleteThred();

};


//******************************************************

class dbDataThread : public QThread
{
     Q_OBJECT

public:


    dbDataThread(dbOSCFile*, QObject *parent=0);
    void run();

public slots:
   void sendF(QString);

private:

   QTimer* timer;
   dbOSCFile*  dc;
   dbConnector* dcon;

   QMutex  mutex;
   QQueue<QString>  fileQueue;    //очередь файлов для отправки


private slots:
	void send();
};




#endif /* DBOSCFILE_H_ */
