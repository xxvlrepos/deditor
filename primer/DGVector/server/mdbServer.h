/*
 * mdbServer.h
 *
 * Многопоточный сервер для доступа к кесшированным регистрам и ретрансляции
 *
 *  Created on: 23 июня 2014 г.
 *      Author: Usach
 */

#ifndef MDBSERVER_H_
#define MDBSERVER_H_


#include <QtCore>
#include <QTcpServer>
#include <QThread>
#include <QTcpSocket>
#include "mMap.h"
#include <execut.h> //только для сообщений
#include <QDomNode>

class mThread;

class SSoc : public QObject{

  Q_OBJECT

public:
	   QTcpSocket* Socket;
	   dIF  CMD;
	   QByteArray tran;
	   QByteArray res;
	   quint16 port;
	   QString adr;
	   bool closeS;

		SSoc(QTcpSocket* S,mThread *thread);
		~SSoc();

		int read();
		void write();
		bool isOpen(){return Socket->isOpen();}
		bool isBlock(){return block;}


public slots:
		void receive();

protected:
	   bool error;
	   bool block;
	   mThread* parentTr;
	   void write(const QByteArray& r);


protected slots:
    void onReadyRead();
};

//*********************************************************
class socList : protected QList<SSoc*>{
  public:
	socList() : QList<SSoc*>(){}
	~socList(){ clear();}
	SSoc* at(int i){return QList<SSoc*>::at(i);}
	SSoc* at(QTcpSocket* soc){
		  int i=search(soc);
		  if(i>=0) return at(i);
		  return NULL;
	}
	void append(SSoc* t) { QList<SSoc*>::append(t);}
	int search(QTcpSocket* soc){
		for(int i=0;i<size();i++)  {if(QList<SSoc*>::at(i)->Socket==soc)	return i;}
		return -1;
	}
	int search(dIF* dif){
			for(int i=0;i<size();i++)  {if(&(QList<SSoc*>::at(i)->CMD)==dif) return i;}
			return -1;
	}
	int search(SSoc* s){
		for(int i=0;i<size();i++)  {if(QList<SSoc*>::at(i)==s)	return i;}
		return -1;
	}

	void remove(SSoc* soc){
		 int i=search(soc);
		 if(i>=0) delete(takeAt(i));
	}

	void remove(QTcpSocket* tsoc){
			 int i=search(tsoc);
			 if(i>=0) delete(takeAt(i));
	}

	SSoc* take(QTcpSocket* soc){
		  int i=search(soc);
		  if(i>=0) return takeAt(i);
		  return NULL;
	}
	void closeAll(){
		while(size()>0) delete(takeLast());
	}
	bool contains(QTcpSocket* soc){ return search(soc)>=0;}
	int size(){return QList<SSoc*>::size();}
};

//*********************************************************************
class mThread : public QThread
{
    Q_OBJECT

public:

	mThread(mMap* m, QObject *parent=0);

    int getSize(){return SocketList.size();}
    void run();
  //  void stop(QTcpSocket*);
    void appendSocket(QTcpSocket* soc);
    void appendComand(SSoc*);

signals:
    void send(SSoc*);

public slots:
	void stop();
	void extReceive(command*);
	void remove(SSoc*);



private:
   mMap* map;
   socList  SocketList;
   int sz;
   void writeBad(SSoc* s,unsigned char bad);
   void fun04(SSoc*);
   void fun14(SSoc*);
   void retrans(SSoc*);

private	slots:
    void onDisconnected();

};


//*************************************************************************

class mdbServer : public QTcpServer{
	Q_OBJECT

public:

	mdbServer(mMap*, QObject *parent = 0);
	virtual ~mdbServer();
	void start(QDomElement);

	int lport;
	int maxNumC;
	QHostAddress haddr;
 signals:
	void error(QTcpSocket::SocketError socketError);

public slots:
	void stop();
	void deleteThred();


protected:
	mMap* map;
    virtual void incomingConnection(qintptr socketDescriptor);
    bool work;

    QDomElement ser;//

    struct dia{
    	dia(quint32 a){ first=last=a;}
    	dia(quint32 a,quint32 b){
    		if(a<b) {first=a;last=b;}
    		else    {first=b;last=a;}
    	}
    	quint32 first;
    	quint32 last;
    };
    QList<dia> allowedIP;
    mThread*   ThredRec;

};

#endif /* MDBSERVER_H_ */
