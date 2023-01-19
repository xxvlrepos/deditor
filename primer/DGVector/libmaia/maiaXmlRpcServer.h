/*
 * libMaia - maiaXmlRpcServer.h
 * Copyright (c) 2007 Sebastian Wiedenroth <wiedi@frubar.net>
 *                and Karl Glatz
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MAIAXMLRPCSERVER_H
#define MAIAXMLRPCSERVER_H

#include <QtCore>
#include <QtXml>
#include <QtNetwork>
#include "maiaFault.h"
#include "maiaObject.h"
#include "maiaCore.h"

class MaiaXmlRpcServer;

class connectThread : public QThread
{
    Q_OBJECT

public:
    QString addr,port;
    connectThread(QTcpSocket* tcpSoc, QObject *parent=0);
    ~connectThread();
    void run();

signals:
    void callMethod(const QString&,int);
    void debug(const QString&);

public slots:
	void stop();
	void sendResponse(const QString&,int);




private:
	  struct DS{
		  bool head;
		  bool chunc;
		  uint cou;
		  QStringList data;
		  QHash<int,QByteArray> rdataH;
		  DS(bool he=false,bool ch=false,int co=0){ head=he; chunc=ch; cou=co; }
		  ~DS(){data.clear(); rdataH.clear();}
	  };


	MaiaXmlRpcServer *server;
    QTcpSocket*  Socket;
    QByteArray headerArr,dataArr;
    QHttpRequestHeader *header;
    QTimer *timerWait;
    int timeLife;
    int szdat;

    DS* CPack;

 private slots:
    	void readFromSocket();

};

//***********************************************
class serverHand : public QThread
{
    Q_OBJECT

public:
	serverHand(MaiaXmlRpcServer* srv, QObject *parent=0);
signals:
	void sendResponse(const QString&,int);

public slots:
	void callMethod(const QString&,int);

private:
	MaiaXmlRpcServer *server;
};

//************************************************************************************************

class MaiaXmlRpcServer : public QTcpServer {
	Q_OBJECT
	
	public:

	    bool debugC;

		MaiaXmlRpcServer(const QHostAddress &address = QHostAddress::Any, quint16 port = 8080, QObject* parent = 0);
		MaiaXmlRpcServer(const QHostAddress &address = QHostAddress::Any, quint16 port = 8080, QList<QHostAddress> *allowedAddresses = 0, QObject *parent = 0);
		MaiaXmlRpcServer(quint16 port = 8080, QObject* parent = 0);

		virtual ~MaiaXmlRpcServer();
		void addMethod(QString method, QObject *responseObject, const char* responseSlot);
		void removeMethod(QString method);

	signals:
		void error(QTcpSocket::SocketError socketError);

	public slots:
		void getMethod(QString method, QObject **responseObject, const char** responseSlot);
		void stop();
		virtual void debug(const QString& str) { qDebug()<<str;}


	protected:

#if QT_VERSION >= 0x050000
		virtual void incomingConnection(qintptr  socketDescriptor);
#else
		virtual void incomingConnection(int socketDescriptor);
#endif

	 signals:
		     void stopAllThread();

	protected slots:
	    void deleteThread();
	
	private:
		  static 	bool invokeMethodWithVariants(QObject *obj,
			  				        			  const QByteArray &method,
			  									  const QVariantList &args,
			  				                      QVariant *ret,
			  									  Qt::ConnectionType type = Qt::AutoConnection);
		  static 	QByteArray getReturnType(const QMetaObject *obj,
			  								 const QByteArray &method,
			  								 const QList<QByteArray> argTypes);


	    void delThread(connectThread*);
	    void start(const QHostAddress& addr, quint16 port, QList<QHostAddress>* allowedAddr=NULL);

		QMutex mutex;
		QList<connectThread*> connectThredList;
		serverHand*           serverH;
		QHash<QString, QPair<QObject*,const char*> > objectMap;
		QList<QHostAddress> *allowedAddresses;

		friend void serverHand::callMethod(const QString&,int);
};

#endif
