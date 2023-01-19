/*
 * libMaia - maiaXmlRpcClient.h
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

#ifndef MAIAXMLRPCCLIENT_H
#define MAIAXMLRPCCLIENT_H

#include <QtCore>
#include <QtXml>
#include <QtNetwork>
#include "maiaCore.h"
#include "maiaObject.h"
#include <dgserver_global.h>

class DGSERVER_EXPORT MaiaXmlRpcClient;


//********************************************************************
class clientThread : public QThread
{
    Q_OBJECT

public:

    clientThread(const QHttpRequestHeader& h,MaiaObject* cl,uint lt=0,QObject *parent=0);
    ~clientThread();
    void run();
    qint16 getLocalPort(){ return socket->localPort();}

signals:
   void debug(const QString&);


public slots:
    bool appSend(MaiaObject*);
	void stop();
	virtual void error(QAbstractSocket::SocketError err);

private:
	uint lTime,sTime;
	QUrl url;
	QTcpSocket *socket;
	QHttpRequestHeader  *shead;
	QHttpResponseHeader *header;
	QQueue<MaiaObject*> queMObj;
	QQueue<MaiaObject*> queCall;
	MaiaObject*         call;
	QString headerString;
	QString dataString;

	QMutex mutex;
	QTimer* liveTimer;

	int  szdat;
	int  status;  //0-нет соед, 1-установлено, 2-разрываем
	bool single;
	bool chunc;
	bool endT; //признак завершения связи


 signals:
	  void  send();
	  void halt();

private	slots:
  void sendData();
  void readSlot();
  void connectSlot();
  void endTime();

};


//**************************************************************************************



class  MaiaXmlRpcClient : public QObject {
	Q_OBJECT
	
	public:
		MaiaXmlRpcClient(QObject* parent = 0);
		MaiaXmlRpcClient(QUrl url, QObject* parent = 0);
		MaiaXmlRpcClient(QUrl url, QString userAgent, QObject *parent = 0);
		~MaiaXmlRpcClient();
		void setUrl(QUrl url);
		QUrl getUrl(){return sUrl;}
		void setUserAgent(QString userAgent);
		clientThread* call(const QString& method,
						   QList<QVariant> args,
						   QObject* responseObject, const char* responseSlot,
						   QObject* faultObject,    const char* faultSlot,
						   uint lifeTime=0,
						   clientThread* oldThread=0);

	public slots:
	    void stop();
		virtual void debug(const QString&);

	protected:
	      clientThread* newThread(MaiaObject* call, uint lifeTime);
		  void deleteThread(clientThread*);


	protected slots:
			void deleteThread();

	private:
		void init();

		QMutex mutex;

		QList<clientThread*> threadList;
		QUrl sUrl;
		QHttpRequestHeader  header;

};

#endif
