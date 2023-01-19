/*
 * dataCollector.h
 *
 * Данный класс сравнивает по списку значения регистров модбас
 *  и в случае изменения  генерирует  запрос для отправки в базу данных
 *
 *  Created on: 30 окт. 2014 г.
 *      Author: Usach
 */

#ifndef DATACOLLECTOR_H_
#define DATACOLLECTOR_H_

#include "mMap.h"
#include <const.h>
#include <QDomDocument>
#include <QDomNode>
#include <QThread>
#include "dbConnector.h"
#include "dataTableMySQL.h"
#include "dataTablePostgreSQL.h"

class dbThread;




class dataCollector :  public QObject {

	 Q_OBJECT

public:
	 struct sendD{
			unsigned char  mdb;
			unsigned short adr;
			short          val;
			QString sm,sa,sv;
			sendD(unsigned char m,unsigned short a ,short v){
				mdb=m;
				adr=a;
				val=v;
				sm.setNum(m,10);
				sa.setNum(a,10);
				sv.setNum(v,10);
			}
	};

	struct sendSTAT{
	 			unsigned char  mdb;
	 			bool          stat;
	 			QString sm,ss;
	 			sendSTAT(unsigned char m,bool s){
	 				mdb=m;
	 				stat=s;
	 				sm.setNum(m,10);
	 				if(stat) ss="1";
	 				else     ss="0";
	 			}
	 };

	dataCollector(QString,mMap* md , QDomDocument doc,QString& type, QObject *parent=0);
	virtual ~dataCollector();
	bool getStatus(){return (bool)thread;};


	bool dbConnect(QDomElement sElem);
	void iniTable();
	void sendQury();//выполняет очередь запросов

	const QString& getNameConn(){return nameCon;};
	QDomElement    getParConn() {return parConn; };


//из потока
	void sendDbDat();
	void clearHistT();
	void closeACT(); //закрыть статусы - при выключении



public slots:
     void start();
 	 void stop();
 	 void genStatic(int);
 	 void genDinamic(int);

protected slots:
   void deleteThred();



protected:

    struct teg{
    	teg(unsigned short a,short v=0,bool uns=false) {addr=a; value=v;};
    	short value;
    	unsigned short addr;
    	unsigned short id;
    	unsigned char  num;
    	QChar          kType;
    	//bool           unsig;
    };

	class mdCan : public QHash<unsigned short,teg>{
	   public:
		iterator insert(unsigned short a,short v){
			return QHash::insert(a,teg(a,v));
		}
		iterator insert(unsigned short a,teg t){
			  t.addr=a;
			  return QHash::insert(a,t);
		}
	};
	class mData : public QHash<unsigned char,mdCan* >{

		public:
		virtual ~mData(){clear();}
		void clear(){
			 QList<unsigned char> k=keys();
			 for(int i=0; i<k.size();i++)   delete(value(k[i]));
			 QHash<unsigned char,mdCan* >::clear();
		}
		int remove(const unsigned char& key){
			delete(value(key));
			return QHash<unsigned char,mdCan* >::remove(key);
		}
	};
	QQueue<QString> queQuery;
	QMutex mutex;
	QMutex mutexQue;



	mData                      memDat;//храним предыдущие значения
	QHash<unsigned short,bool> actData; //храним статус активности

	QString      nameCon;
	QDomElement  parConn;


	private:
	mMap* modbusDat;

	int deepHis;//Глубина хранения истории
    dbThread* thread;

	QDomElement rootNode;
	QDomDocument xmldoc;

	DBTable* tableSER;  //таблица активности сервера
	DBTable* tableACT;  //таблица активности приборов
	DBTable* tableTD;   //таблица оперативных данных
	DBTable* tableHD;   //таблица истории
	DBTable* tableSTD;  //таблица статических данных
	DBTable* tableDITD; //словарь тегов
	DBTable* tableOPT;  //описание тегов

	DBTriger* updTriger;
	DBProc*   clearProc;


	void createStaticDT(int,const QStringList&);
	void genDataList(int,const QStringList&);

	void addInQue(const QStringList&);
	void testData(unsigned char,QList<sendD>&);
	QList<dataCollector::sendD>                getListData();
	QList<dataCollector::sendSTAT>             getListStat();

	signals:
	    void stopAll();

};

//--------------------------------------------------------------------------------------

class dbThread: public QThread
{
     Q_OBJECT

public:
     static const int clearTime=60;

     dbThread(dataCollector* d, QObject *parent=0):QThread(parent){
    	dc=d;
    };

    void run(){
    	counClear=0;
    	QString dbType;
    	dcon=new dbConnector(dc->getNameConn());
    	if(!dcon->dbConnect(dc->getParConn(),dbType)) return;

    	dc->iniTable();
        timer=new QTimer();
        timer->setSingleShot(false);
        timer->setInterval(1000);
        connect(timer, SIGNAL(timeout()), this, SLOT(setData()),Qt::DirectConnection);
        timer->start();
        exec();
    };


public slots:
   void halt(){
	   timer->stop();
	   dc->closeACT();
	   dcon->dbClose();
	   quit();
   }


private:
    dataCollector* dc;
    dbConnector* dcon;

    QTimer* timer;
    int counClear;


private slots:
		void setData(){
			dc->sendQury();
			dc->sendDbDat();
			if(++counClear==clearTime) {counClear=0;dc->clearHistT();}
		}
};

#endif /* DATACOLLECTOR_H_ */
