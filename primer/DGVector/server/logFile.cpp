/*
 * logFile.cpp
 *
 *  Created on: 11 февр. 2015 г.
 *      Author: Usach
 */

#include "logFile.h"


//***************************************************************************

	QMutex       logFile::mutLOG;
    QQueue<QPair<QDateTime,QString> > logFile::logBuff;
    logFile*     logFile::logF=NULL;
    QDateTime    logFile::endDT=QDateTime::currentDateTime();



logFile::logFile(const QString &name, QObject *parent) : QObject(parent) {
		LogFileName=name;
		thread=NULL;
		logF=this;

}

logFile::~logFile(){
	thread->deleteLater();
	logF=NULL;
}


void logFile::openLog() {
	if(thread==NULL){
		thread=new LogThread(LogFileName);
		QObject::connect(getInstance(),SIGNAL(writeD(QString)), thread, SLOT(addToFile(QString)));
		thread->moveToThread(thread);
		thread->start();
#if QT_VERSION >= 0x050000
		qInstallMessageHandler(logOutput5);
#else
		qInstallMsgHandler(logOutput);
#endif
	}
}

#if QT_VERSION >= 0x050000
void logFile::logOutput5(QtMsgType type,const QMessageLogContext &context, const QString& msg){
	logOutput(type,msg.toLocal8Bit().constData());
}
#endif

void logFile::logOutput(QtMsgType type, const char *msg)
{
	int i=0;

	while(msg[i]!=0 && i<maxLenStr) i++;
	if(i==maxLenStr) return;

	mutLOG.lock();
    QDateTime curDT=QDateTime::currentDateTime();
    if(curDT>endDT) endDT=curDT;
    else            endDT=endDT.addMSecs(1);

	QString deb(endDT.toString("yyyyMMddThh:mm:ss.zzz"));
    switch (type)
    {
		case QtDebugMsg:    deb.append("[D] "); break;
		case QtWarningMsg:  deb.append("[W] "); break;
		case QtCriticalMsg: deb.append("[C] "); break;
		case QtFatalMsg:    deb.append("[F] "); break;
    }
    deb.append(msg);
    logBuff.enqueue(QPair<QDateTime,QString>(endDT,deb));
    while(logBuff.size()>maxBufSz) {logBuff.dequeue();}
    getInstance()->emitSignal(deb);
    mutLOG.unlock();
    if ( type==QtFatalMsg)  abort();

}

QStringList logFile::getLog(QDateTime startDT){
	QStringList list;
	mutLOG.lock();
	for(int i=logBuff.size()-1;i>=0;i--){
		if(logBuff[i].first>startDT) list.prepend(logBuff[i].second);
		else break;
	}
	mutLOG.unlock();
	return list;
}

