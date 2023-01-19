/*
 * libMaia - maiaXmlRpcClient.cpp
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

#include "maiaXmlRpcClient.h"
#include "maiaFault.h"
#include "maiaCore.h"

clientThread::clientThread(const QHttpRequestHeader& h,MaiaObject* cl,uint lt,QObject *parent) : QThread(parent){
    	shead=new QHttpRequestHeader(h);
    	url.setUrl(shead->path());
    	if(lt>maiaCore::timeLifeMax) lTime=maiaCore::timeLifeMax;
    	else lTime=lt; //время жизни соединения
    	sTime=0;
    	socket=NULL;
    	header=NULL;
    	call=NULL;

    	single=false;
    	status=0;
    	szdat=0;
    	chunc=false;
    	endT=false;

    	if(lTime>0){
    		QString txt;
    		shead->setValue("Connection","Keep-Alive");
    		shead->setValue("Keep-Alive","timeout="+txt.setNum(lTime));
    	}

    	queMObj.enqueue(cl);
}

clientThread::~clientThread(){
	delete(shead);
	if(header) delete(header);

}

 void clientThread::run(){
	    liveTimer=new QTimer();
	    liveTimer->setSingleShot(true);
    	socket = new  QTcpSocket(this);


    	connect(this, SIGNAL(send()), this ,SLOT(sendData()),Qt::QueuedConnection);
    	connect(this, SIGNAL(halt()), this ,SLOT(stop()),Qt::QueuedConnection);
    	connect(liveTimer, SIGNAL(timeout()), this, SLOT(endTime()));
		connect(socket, SIGNAL(readyRead()), this, SLOT(readSlot()));
		connect(socket, SIGNAL(disconnected()), this, SIGNAL(halt()));

		connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(error(QAbstractSocket::SocketError)));
		connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(stop()));



		QTimer::singleShot(0,this,SLOT(connectSlot()));
		exec();

		queMObj.clear();
	  	queCall.clear();
 }

 void clientThread::connectSlot(){
	 socket->connectToHost(url.host(),url.port(0),QIODevice::ReadWrite);
	 if(!socket->waitForConnected(3000)){
		 if(!queMObj.isEmpty()) queMObj.first()->parseResponse();
		 stop();
		 return;
	 }
	 status=1;
	 emit send();
 }

 void  clientThread::error(QAbstractSocket::SocketError err){
	 if(err!=QAbstractSocket::RemoteHostClosedError) emit debug(socket->errorString());
 }

 void clientThread::endTime(){
	 mutex.lock();
	    if(queMObj.isEmpty() && queCall.isEmpty() && !call) 	status=2;
	 mutex.unlock();
	 if(status==2) emit halt();
 }

 void clientThread::stop(){
	mutex.lock();
	    endT=true;  status=2;
	mutex.unlock();
	disconnect(socket,SIGNAL(disconnected()), this, SLOT(stop()));
  	if(socket->isOpen()) socket->close();
  	quit();
 }


bool clientThread::appSend(MaiaObject* mCall){

	QMutexLocker mutexloc(&mutex);
	if(single || status>1 || endT)  return false;
    queMObj.enqueue(mCall);
    if(lTime==0) single=true;
    //if(status==1 && !queCall.size() && !call)
    emit send();
	return true;
 }


 void clientThread::sendData(){

	  QMutexLocker mutexloc(&mutex);
	  if(status!=1 || queMObj.isEmpty() || call) return;
	  //неготово соед или пустая очередь или тек посылка обрабатывается
	  liveTimer->stop();
	  while(!queMObj.isEmpty()) queCall.enqueue(queMObj.dequeue()); //копируем запросы
	  mutexloc.unlock();


	  QByteArray body;
	  QString txt;
	  QHttpRequestHeader head(*shead);

	  call=queCall.first();
	  if(queCall.size()==1){
		  body.append(call->prepareCall().toUtf8());
		  head.setContentLength(body.size());
		  body.prepend(head.toString().toUtf8());
	  }else{
		  head.addValue("Transfer-Encoding","chunked");
		  body.prepend(head.toString().toUtf8());
		  for(int i=0; i<queCall.size();i++){
			  QByteArray  block(queCall[i]->prepareCall().toUtf8());
			  body.append(txt.setNum(block.size(),16));
			  body.append("\r\n");
			  body.append(block);
			  body.append("\r\n");
		  }
		  body.append("0\r\n\r\n");
	  }

	// qDebug()<<body;
	  socket->write(body);
	  socket->flush();
 }

 void clientThread::readSlot(){
	 QByteArray lastLine;
	 bool ok;

	 while (socket->canReadLine()) {
		lastLine = socket->readLine();
		if(!header){
			if(lastLine != "\r\n") headerString.append(QString::fromUtf8(lastLine));
			else { /* http header end */
				if(headerString.size()==0) continue;
				sTime=0;
				szdat=0;
				chunc=false;
				//qDebug()<<headerString;
				header = new QHttpResponseHeader(headerString);
				if(!header->isValid()) {
					/* return http error */
					emit debug("Invalid Header");
					emit halt();
					return;
				}
				szdat=maiaCore::getContLeng(header);
				if(szdat==0){
					emit debug("No Content Length");
					emit halt();
					return;
				}else{
					if(szdat<0) chunc=true; //нарезка
				    else        call=queCall.dequeue();
				}
				sTime=maiaCore::getKepTime((QHttpHeader*)header);
			}

		}else{
			if(szdat<=0){
				if(lastLine.trimmed().size()>0){
					szdat=QString::fromUtf8(lastLine).trimmed().toUInt(&ok,16);
					if(!ok)   stop();
					if(szdat>0){
						call=queCall.dequeue();
						continue;
					}
				}else continue;
			}

	        if(szdat>0){
	        	call->response.append(QString::fromUtf8(lastLine));
				if(szdat==call->response.size()) {
						/* all data complete */
		        		call->parseResponse();
						szdat=0;
						if(chunc) continue;
				}

	        }
	        if(szdat==0){
	        	headerString.clear();
	        	delete(header);
	        	header=NULL;

	        	mutex.lock();
	        	call=NULL;
				if(single){ status=2; socket->disconnectFromHost();}
				else{
					if(!queMObj.isEmpty())   emit send();
					else{
						if(endT){  status=2; socket->disconnectFromHost();}
						else 	liveTimer->start(sTime);
					}
				}
				mutex.unlock();

	        }
		}
	 }
 }


//***************************************************************************

MaiaXmlRpcClient::MaiaXmlRpcClient(QObject* parent) : QObject(parent)
{
	init();
}

MaiaXmlRpcClient::MaiaXmlRpcClient(QUrl url, QObject* parent) : QObject(parent)
{
	setUrl(url);
}

MaiaXmlRpcClient::MaiaXmlRpcClient(QUrl url, QString userAgent, QObject *parent) : QObject(parent) {
	// userAgent should adhere to RFC 1945 http://tools.ietf.org/html/rfc1945
	setUrl(url);
#if QT_VERSION < 0x050000
	header.setValue("User-Agent", userAgent.toAscii());
#else
	header.setValue("User-Agent", userAgent.toLatin1());
#endif

}

MaiaXmlRpcClient::~MaiaXmlRpcClient(){
	stop();
}

void MaiaXmlRpcClient::init() {

	header=QHttpRequestHeader();
	header.setRequest("POST",sUrl.toString());
	header.addValue("User-Agent", "libmaia/0.2");
	header.setContentType ("text/xml");
	header.addValue("Connection","close");
}

void MaiaXmlRpcClient::setUrl(QUrl url) {
	if(!url.isValid()) 	return;
	sUrl.swap(url);
	init();
}

void MaiaXmlRpcClient::setUserAgent(QString userAgent) {
#if QT_VERSION < 0x050000
	header.setValue("User-Agent", userAgent.toAscii());
#else
	header.setValue("User-Agent", userAgent.toLatin1());
#endif
}

clientThread* MaiaXmlRpcClient::newThread(MaiaObject* call, uint lifeTime){

	clientThread* cThread  = new clientThread(header,call,lifeTime);
	mutex.lock();
	threadList.append(cThread);
	connect(cThread, SIGNAL(finished()), this, SLOT(deleteThread())); //???
	connect(cThread, SIGNAL(debug(const QString&)), this, SLOT(debug(const QString&)));
	cThread->moveToThread(cThread);
	mutex.unlock();
	cThread->start();
	return cThread;
}

void MaiaXmlRpcClient::debug(const QString& t){
	clientThread* thread = qobject_cast<clientThread*>(sender());
	QString txt;
	txt.setNum(thread->getLocalPort());
	qDebug()<<txt+":"+t;
}

clientThread* MaiaXmlRpcClient::call(const QString& method, QList<QVariant> args,
							QObject* responseObject, const char* responseSlot,
							QObject* faultObject, const char* faultSlot,
							uint lifeTime,
							clientThread* oldThread) {
	MaiaObject* call = new MaiaObject(method,args,this);
	connect(call, SIGNAL(aresponse(const QVariant&, const QString&)), responseObject, responseSlot);
	connect(call, SIGNAL(fault(int, const QString&, const QString&)), faultObject, faultSlot);

    if(oldThread){
		QMutexLocker locker(&mutex);
		if(threadList.contains(oldThread)){
			if(oldThread->appSend(call)) return oldThread;
		}
		locker.unlock();
	}
	return newThread(call,lifeTime);
}


void MaiaXmlRpcClient::deleteThread(clientThread* thread){
	    if(thread){
	    	mutex.lock();
	    	threadList.removeAt(threadList.indexOf(thread));
	    	mutex.unlock();
			thread->wait();
			delete(thread);
	    }
}

void MaiaXmlRpcClient::deleteThread(){
	clientThread* thread = qobject_cast<clientThread*>(sender());
	deleteThread(thread);
}


void MaiaXmlRpcClient::stop(){

	mutex.lock();
	while(threadList.size()>0){
		clientThread* thread=threadList.takeLast();
		thread->stop();
		thread->wait();
		delete(thread);
	}
	mutex.unlock();
}






