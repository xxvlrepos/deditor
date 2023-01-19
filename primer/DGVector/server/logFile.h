/*
 * logFile.h
 *
 *  Created on: 11 февр. 2015 г.
 *      Author: Usach
 */

#ifndef SERVER_LOGFILE_H_
#define SERVER_LOGFILE_H_

#include <QtCore>
#include <QObject>
#include <QTextCodec>
#include <QTextStream>
#include <dgserver_global.h>
#include <QThread>

class DGSERVER_EXPORT logFile;
class LogThread;

class  logFile : public QObject{

	Q_OBJECT

public:

    static const int    maxBufSz=100;
    static const int    maxLenStr=40;
	static QMutex       mutLOG;
	static QQueue<QPair<QDateTime,QString> > logBuff;
	static logFile* logF;
	static QDateTime endDT;

	logFile(const QString &name, QObject *parent=0);
	~logFile();
	static logFile* getInstance(){return logF;};
	static void logOutput(QtMsgType type, const char *msg);
#if QT_VERSION >= 0x050000
    static void logOutput5(QtMsgType type,const QMessageLogContext &context, const QString& msg);
#endif
	void openLog();

signals:
   void writeD(QString);

public slots:

    QStringList getLog(QDateTime);


protected:
    void emitSignal(QString dat){  emit writeD(dat);}
	LogThread* thread;
	QString   LogFileName;
};


//**********************************************************
class LogThread: public QThread
{
	Q_OBJECT
public:
	   static const int maxSz=1024*1024;
	   LogThread(const QString &name, QObject *parent=0):QThread(parent){
		   LogFileName=name;
	   }
	   void run(){
		    outC = new  QTextStream();
			#ifdef _WIN_
				codecName = "CP1251";
			#else
				codecName = "UTF-8";
			#endif
			file=new QFile(LogFileName);
			mode=QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append;
			outC->setDevice(file);
			outC->setCodec(codecName);
			file->open(mode);
		    exec();
	   }
public	slots:
	void addToFile(QString d){
		if(file->isOpen()){
			if(testSize()) *outC<<d<<endl;
		}
	}

private:
	const char* codecName;
	QFile* file;
	QTextStream* outC;
	QIODevice::OpenMode mode;
	QString   LogFileName;

	bool testSize (){
		if(file->size()>=maxSz){
			QString cf,fn;
			fn=file->fileName();
			int i=fn.lastIndexOf("/");
			cf=fn.left(i)+"/old_"+fn.right(fn.size()-i-1);
			if(QFile::exists(cf)) QFile::remove(cf);
			file->copy(cf);
			file->close();
			QFile::remove(fn);
			return file->open(mode);
		}
		return true;
	}
};


#endif /* SERVER_LOGFILE_H_ */
