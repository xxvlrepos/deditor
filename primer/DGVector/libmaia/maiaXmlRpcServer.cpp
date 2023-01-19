/*
 * libMaia - maiaXmlRpcServer.cpp
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

#include "maiaXmlRpcServer.h"
#include "maiaFault.h"
#include "maiaCore.h"

connectThread::connectThread(QTcpSocket* tcpSoc, QObject *parent) : QThread(parent){
    	Socket=tcpSoc;
	    header = NULL;
	 	timeLife=0;
	 	szdat=0;
	 	CPack=NULL;
	 	timerWait=NULL;
}

connectThread::~connectThread(){
	// if(Socket!=NULL)    delete(Socket);
	// if(timerWait!=NULL) delete(timerWait);
}

 void connectThread::run(){

	    addr=Socket->peerAddress().toString();
	    port=port.setNum(Socket->peerPort());
	    if(timerWait==NULL)timerWait=new QTimer();
	    timerWait->setSingleShot(true);
	    connect(timerWait, SIGNAL(timeout()), this, SLOT(stop()));
	    connect(Socket,SIGNAL(readyRead()), this, SLOT(readFromSocket()),Qt::DirectConnection);
    	connect(Socket,SIGNAL(disconnected()),this,SLOT(stop()));
  		exec();
  		if(Socket!=NULL)    delete(Socket);
  		if(timerWait!=NULL) delete(timerWait);
}


 void connectThread::stop(){
	    disconnect(Socket,SIGNAL(disconnected()),this,SLOT(stop()));
	    if(Socket->isOpen())	Socket->close();
	 	quit();
}

 void connectThread::readFromSocket() {
 	QByteArray lastLine;
 	bool ok;
    bool chunc;

 	if(timerWait->isActive()) timerWait->stop();
 	while(Socket->canReadLine()) {
 		lastLine = Socket->readLine();
 		if(!header){
 			    if(lastLine != "\r\n") headerArr.append(lastLine);
 			    else { /* http header end */
 					if(headerArr.size()==0) continue;
 					szdat=0;
 					chunc=false;
 					header = new QHttpRequestHeader(QString::fromUtf8(headerArr));
 					//qDebug()<<headerArr;
 					if(!header->isValid()) {
 						/* return http error */
 						emit debug("Invalid Header");
 						stop();
 					}else if(header->method() != "POST") {
 						/* return http error */
 						emit debug("No Post!");
 						stop();
 					}else{
 						szdat=maiaCore::getContLeng(header);
 						if(szdat==0){
 							emit debug("No Content Length");
 							stop();
 						}else if(szdat<0) 	chunc=true;
 					}

 					timeLife=maiaCore::getKepTime((QHttpHeader*)header);
 					if(timeLife>maiaCore::timeLifeMax) timeLife=maiaCore::timeLifeMax;
 					if(CPack){
 						emit debug("Not completed in the previous translate");
 						stop();
 					}
 					CPack=new DS(true,chunc,0);
 				}

 		}else{
 			if(szdat<=0){
 				if(lastLine.trimmed().size()>0){
					szdat=lastLine.trimmed().toUInt(&ok,16);
					if(!ok)   stop();
					if(szdat) continue;
 				}else continue;
 			}
 			if(szdat>0){
				dataArr.append(lastLine);
				if(szdat == dataArr.size()) {
					/* all data complete */
					//qDebug()<<dataArr;
					CPack->rdataH[CPack->cou]=QByteArray();
					CPack->cou++;
					CPack->data<<QString::fromUtf8(dataArr);
					dataArr.clear();
					szdat=0;
					if(CPack->chunc) continue;
				}
 			}

 			if(szdat==0){
 				headerArr.clear();
 				delete(header);
 				header=NULL;
 				for(uint i=0; i<CPack->cou;i++){
 					emit callMethod(CPack->data[i],i);
 				}
 			}
 		}
 	}
 }


 void connectThread::sendResponse(const QString& content,int num) {


	if(CPack==NULL) return;
	QString txt;

	QByteArray block(content.toUtf8());



	if(CPack->head){
		CPack->head=false;
		QHttpResponseHeader rheader(200, "Ok");
		rheader.setValue("Server", "MaiaXmlRpc/0.1");
		rheader.setValue("Content-Type", "text/xml");
		if(timeLife>0){
			QString txt;
			rheader.setValue("Connection","Keep-Alive");
			rheader.setValue("Keep-Alive","timeout="+txt.setNum(timeLife));
		}else{
			rheader.setValue("Connection","close");
		}
		if(CPack->chunc){
			rheader.addValue("Transfer-Encoding","chunked");
			CPack->rdataH[num].append(txt.setNum(block.size(),16));
			CPack->rdataH[num].append("\r\n");
			CPack->rdataH[num].append(block);
			CPack->rdataH[num].append("\r\n");
		}else {
			rheader.setContentLength(block.size());
			CPack->rdataH[num]=block;
		}
		CPack->cou--;
		CPack->rdataH[0].prepend(rheader.toString().toUtf8());
	}else{
		CPack->rdataH[num].append(txt.setNum(block.size(),16));
		CPack->rdataH[num].append("\r\n");
		CPack->rdataH[num].append(block);
		CPack->rdataH[num].append("\r\n");
		CPack->cou--;
		if(CPack->cou==0){
			CPack->rdataH[CPack->data.size()-1].append("0\r\n\r\n"); //закрыли последний
		}

	}
//	qDebug()<<rdata;
	for(int i=0;i<CPack->data.size();i++){
		if(CPack->rdataH.contains(i)){
			if(CPack->rdataH[i].size()==0) break; //еще не заполнен
			Socket->write(CPack->rdataH[i]);
			Socket->flush();
			CPack->rdataH.remove(i);
		}
	}
	if(CPack->cou==0){
		delete(CPack);
		CPack=NULL;
		if(timeLife>0 ) timerWait->start(timeLife+500);
		else            Socket->disconnectFromHost();
	}
 }

//***************************************************************************
//***************************************************************************

 serverHand ::serverHand (MaiaXmlRpcServer* srv, QObject *parent) : QThread(parent){
	 server=srv;
 }

void serverHand ::callMethod(const QString& call,int num){

	   connectThread* cthread = qobject_cast<connectThread*>(sender());
	   // qDebug()<<cthread<<call;

	    QDomDocument doc;
		QList<QVariant> args;
		QVariant ret;
		QString response;
		QObject *responseObject;
		const char *responseSlot;

		if(!doc.setContent(call)) { /* recieved invalid xml */
			MaiaFault fault(-32700, "parse error: not well formed");
			QMetaObject::invokeMethod(cthread,"sendResponse",Qt::QueuedConnection,Q_ARG(QString,fault.toString()),Q_ARG(int,num));
			return;
		}

		QDomElement methodNameElement = doc.documentElement().firstChildElement("methodName");
		QDomElement params = doc.documentElement().firstChildElement("params");

		if(methodNameElement.isNull()) { /* invalid call */
			MaiaFault fault(-32600, "server error: invalid xml-rpc. not conforming to spec");
			QMetaObject::invokeMethod(cthread,"sendResponse",Qt::QueuedConnection,Q_ARG(QString,fault.toString()),Q_ARG(int,num));
			return;
		}

		QString methodName = methodNameElement.text();

		server->getMethod(methodName, &responseObject, &responseSlot);
		if(!responseObject) { /* unknown method */
			MaiaFault fault(-32601, "server error: requested method not found");
			QMetaObject::invokeMethod(cthread,"sendResponse",Qt::QueuedConnection,Q_ARG(QString,fault.toString()),Q_ARG(int,num));
			return;
		}

		QDomNode paramNode = params.firstChild();
		//qDebug()<<paramNode.nodeName();
		while(!paramNode.isNull()) {
			args << MaiaObject::fromXml( paramNode.firstChild().toElement());
			paramNode = paramNode.nextSibling();
		}


		if(!server->invokeMethodWithVariants(responseObject, responseSlot, args, &ret,Qt::DirectConnection)) { /* error invoking... */
			MaiaFault fault(-32602, "server error: invalid method parameters");
			QMetaObject::invokeMethod(cthread,"sendResponse",Qt::QueuedConnection,Q_ARG(QString,fault.toString()),Q_ARG(int,num));
			return;
		}


		if(ret.canConvert<MaiaFault>()) {
			response = ret.value<MaiaFault>().toString();
		} else {
			response = MaiaObject::prepareResponse(ret);
		}
		QMetaObject::invokeMethod(cthread,"sendResponse",Qt::QueuedConnection,Q_ARG(QString,response),Q_ARG(int,num));

}
//****************************************************************************
//****************************************************************************

MaiaXmlRpcServer::MaiaXmlRpcServer(const QHostAddress &address, quint16 port, QObject* parent) : QTcpServer(parent) {
	start(address, port);
}

MaiaXmlRpcServer::MaiaXmlRpcServer(quint16 port, QObject* parent) : QTcpServer (parent) {
	start(QHostAddress::Any, port);
}

MaiaXmlRpcServer::MaiaXmlRpcServer(const QHostAddress &address, quint16 port, QList<QHostAddress> *allowedAddresses, QObject *parent) : QTcpServer(parent) {
	start(address, port,allowedAddresses);
}

MaiaXmlRpcServer::~MaiaXmlRpcServer(){
    stop();
	while(connectThredList.size()>0){
		delThread(connectThredList.last());
	}
}

void MaiaXmlRpcServer::start(const QHostAddress &addr, quint16 port, QList<QHostAddress>* allowedAddr){

	debugC=false;
	allowedAddresses = allowedAddr;
	serverH=NULL;
	listen(addr, port);
	//qDebug() << serverError() << errorString();
}

void MaiaXmlRpcServer::addMethod(QString method, QObject* responseObject, const char* responseSlot) {
	QMutexLocker locker(&mutex);
	objectMap[method] = QPair<QObject*,const char*>(responseObject,responseSlot);
}

void MaiaXmlRpcServer::removeMethod(QString method) {
	QMutexLocker locker(&mutex);
	objectMap.remove(method);
}

void MaiaXmlRpcServer::getMethod(QString method, QObject **responseObject, const char **responseSlot) {
	QMutexLocker locker(&mutex);
	if(!objectMap.contains(method)) {
		*responseObject = NULL;
		*responseSlot = NULL;
		return;
	}
	*responseObject = objectMap[method].first;
	*responseSlot = objectMap[method].second;
}


#if QT_VERSION >= 0x050000
	void MaiaXmlRpcServer::incomingConnection(qintptr socketDescriptor)
#else
	void MaiaXmlRpcServer::incomingConnection(int socketDescriptor)
#endif
{
	QString txt;
	QTcpSocket* tcpSocket;
	tcpSocket= new QTcpSocket();
	if (!tcpSocket->setSocketDescriptor(socketDescriptor)) {
		    emit error(tcpSocket->error());
		    delete(tcpSocket);
		    return;
	}

	QHostAddress addr=tcpSocket->peerAddress();
//	quint32 iaddr=addr.toIPv4Address();


	if (!this->allowedAddresses || this->allowedAddresses->isEmpty() || this->allowedAddresses->contains(addr)) {

		  if(serverH==NULL){
			  serverH=new serverHand(this);
			  serverH->moveToThread(serverH);
			  serverH->start();
		  }


		   connectThread *cthread = new connectThread(tcpSocket);
		   tcpSocket->moveToThread(cthread); //передаем сокет в поток
		   connectThredList.append(cthread);
		   connect(this, SIGNAL(stopAllThread()), cthread, SLOT(stop()), Qt:: QueuedConnection);
		   connect(cthread, SIGNAL(finished()), this, SLOT(deleteThread()), Qt:: QueuedConnection);
		   connect(cthread, SIGNAL(debug(const QString&)), this, SLOT(debug(const QString&)));

		   connect(cthread, SIGNAL(callMethod(const QString&,int)),    serverH, SLOT(callMethod(const QString&,int)),Qt::QueuedConnection);

		   cthread->moveToThread(cthread);
		   cthread->start();

/*
		MaiaXmlRpcServerConnection *client = new MaiaXmlRpcServerConnection(tcpSocket, this);
		connect(client, SIGNAL(getMethod(QString, QObject **, const char**)),
			this, SLOT(getMethod(QString, QObject **, const char**)));
*/
		if(debugC) qWarning()<<addr.toString()+":" +txt.setNum(tcpSocket->peerPort())+" RPC connect";

	} else {
		qWarning() << "Rejected connection attempt from" << addr.toString();
		//tcpSocket->disconnectFromHost();
		tcpSocket->close();
		delete(tcpSocket);
	}
}

bool MaiaXmlRpcServer::invokeMethodWithVariants(QObject *obj,
			const QByteArray &method, const QVariantList &args,
			QVariant *ret, Qt::ConnectionType type) {

	// QMetaObject::invokeMethod() has a 10 argument maximum
	if(args.count() > 10)
		return false;

	QList<QByteArray> argTypes;
	for(int n = 0; n < args.count(); ++n)
		argTypes += args[n].typeName();

	// get return type
	int metatype = 0;
	QByteArray retTypeName = getReturnType(obj->metaObject(), method, argTypes);
	if(!retTypeName.isEmpty()  && retTypeName != "QVariant") {
		metatype = QMetaType::type(retTypeName.data());
		if(metatype == 0) // lookup failed
			return false;
	}

	QGenericArgument arg[10];
	for(int n = 0; n < args.count(); ++n)
		arg[n] = QGenericArgument(args[n].typeName(), args[n].constData());

	QGenericReturnArgument retarg;
	QVariant retval;
	if(metatype != 0 && retTypeName != "void") {
		retval = QVariant(metatype, (const void *)0);
		retarg = QGenericReturnArgument(retval.typeName(), retval.data());
	} else { /* QVariant */
		retarg = QGenericReturnArgument("QVariant", &retval);
	}

	if(retTypeName.isEmpty() || retTypeName == "void") { /* void */
		if(!QMetaObject::invokeMethod(obj, method.data(), type,
						arg[0], arg[1], arg[2], arg[3], arg[4],
						arg[5], arg[6], arg[7], arg[8], arg[9]))
			return false;
	} else {
		if(!QMetaObject::invokeMethod(obj, method.data(), type, retarg,
						arg[0], arg[1], arg[2], arg[3], arg[4],
						arg[5], arg[6], arg[7], arg[8], arg[9]))
			return false;
	}

	if(retval.isValid() && ret)
		*ret = retval;
	return true;
}

QByteArray MaiaXmlRpcServer::getReturnType(const QMetaObject *obj,
			const QByteArray &method, const QList<QByteArray> argTypes) {
	for(int n = 0; n < obj->methodCount(); ++n) {
		QMetaMethod m = obj->method(n);
#if QT_VERSION < 0x050000
		QByteArray sig = m.signature();
#else
		QByteArray sig = m.methodSignature();
#endif
		int offset = sig.indexOf('(');
		if(offset == -1)
			continue;
		QByteArray name = sig.mid(0, offset);
		if(name != method)
			continue;
		if(m.parameterTypes() != argTypes)
			continue;

		return m.typeName();
	}
	return QByteArray();
}


void MaiaXmlRpcServer::stop(){
	emit stopAllThread();
	if(isListening ()){
		close();
		qWarning()<<"stop RPC server";
	}
}


void MaiaXmlRpcServer::delThread(connectThread* cthread){
	    if(cthread){
	    	connectThredList.removeAt(connectThredList.indexOf(cthread));
	    	if(debugC) qWarning()<<cthread->addr+":"+cthread->port+" RPC disconnect";
	    	cthread->wait();
	    	delete(cthread);
	    }
	   if(connectThredList.size()==0 && serverH!=NULL){
		   serverH->quit();
		   serverH->wait();
		   delete(serverH);
		   serverH=NULL;
	   }
}

void MaiaXmlRpcServer::deleteThread(){
	connectThread* cthread = qobject_cast<connectThread*>(sender());
	delThread(cthread);
}

