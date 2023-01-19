/*
 * dgserver.cpp
 *
 *  Created on: 30 марта 2015 г.
 *      Author: Usach
 */

#include "dgserver.h"



dgserver::dgserver(int argc, char **argv) :QtService<QtSingleCoreApplication>(argc,argv,"DGserver"){
		setServiceDescription("Server to poll devices such as vector");
		setServiceFlags(QtServiceBase::CanBeSuspended);
		//setServiceFlags(QtServiceBase::Default);
		exitBit=false;
		isService=true;
		autoRun=false;
		s=NULL;
		LogF=NULL;
		//разбор параметров
		 QStringList args;
		 for (int i = 0; i < argc; ++i) args.append(QString::fromLocal8Bit(argv[i]));
		 while (args.size() > 1) {
		        QString a =  args.takeAt(1);
		        if (a == QLatin1String("-h") || a == QLatin1String("-help")) {
						printf("\n%s -[i|u|e|t|v|h]\n"
							   "\t-i(nstall) [account] [password]\t: Install the service, optionally using given account and password\n"
							   "\t-u(ninstall)\t: Uninstall the service.\n"
							   "\t-e(xec)\t\t: Run as a regular application. See additional parameters.\n"
							   "\t-t(erminate)\t: Stop the service.\n"
							   "\t-v(ersion)\t: Print version and status information.\n"
							   "\t-h(elp)   \t: Show this help\n"
							   "\tNone of these arguments\t: Start the service.\n"
								"\n"
							   "\tAdditional parameters for run as a regular application -e(xec).\n"
							   "\t-s(tart)\t: Start poll devices.\n"
							   "\t-d(ebug)\t: Enable debugging.\n"
							   "\t-d(ebug)[1-254]\t: Enable full debugging for the device address.\n"
							   "\t-c(ycle)[1-254]\t: Must the actual cycle time. \n"
								"\n"
							    "\tAdditional parameters for COM-port.\n"
								"\t-w[5-50]\t: Delay sending the next request.\n"
								"\t-m[5-50]\t: The maximum gap response modbus.\n",

								args.at(0).toLatin1().constData());
								exitBit=true;
								return;

		        }
		        if (a == QLatin1String("-v") || a == QLatin1String("-version")) {
		        	 printf("The DLL version %s\n",server::ver.toLatin1().constData());
		        	 continue;
		        }
		        if (a == QLatin1String("-e") || a == QLatin1String("-exec")) {
		              isService=false;
		              continue;
		        }
				if (!isService) {
					if (a == QLatin1String("-s") || a == QLatin1String("-start")) {
						autoRun = true;
						continue;
					}
					if (a.left(2) == QLatin1String("-d") || a.left(6) == QLatin1String("-debug")) {
						SYSTEM::Debug = true;
						QString ad;
						if(a.left(6) == QLatin1String("-dedug")){ if(a.size()>6) ad=a.mid(6);}
						else                                    { if(a.size()>2) ad=a.mid(2);}
						if(ad!=""){
							int n=ad.toUShort();
							if(n>0 && n<=247){
								dataServer::NumDebugDev = n;
								SYSTEM::NumDebug=n;
								com485::debugN=n;
								ExeListThread::debugN=n;
							}
						}
						continue;
					}
					if (a.left(2) == QLatin1String("-c") || a.left(6) == QLatin1String("-cycle")) {
						QString ad;
						if(a.left(6) == QLatin1String("-cycle")){ if(a.size()>6) ad=a.mid(6);}
						else                                    { if(a.size()>2) ad=a.mid(2);}
						if(ad!=""){
							dataServer::NumDebugCycle = (uchar) ad.toUShort();
						}
						continue;
					}
					if (a.left(2) == QLatin1String("-w")) {
						QString ad;
					    if(a.size()>2) ad=a.mid(2);
						if(ad!=""){
							uint p=ad.toUInt();
							if(p>=5 && p<=50) execut::comDelay =p;
						}
						continue;
					}
					if (a.left(2) == QLatin1String("-m")) {
						QString ad;
						if(a.size()>2) ad=a.mid(2);
						if(ad!=""){
							uint p=ad.toUInt();
							if(p>=5 && p<=50) execut::comMWait =p;
						}
						continue;
					}
				}
		 }



}

dgserver::~dgserver(){
	 if(s) s->stop();
}

void   dgserver::start(){

	   int rez=testApp();
	   if(rez){ application()->exit(rez); 	return; }
       if(!s){
    	   initDG();
    	   s=new server(&MDBArray,application());
         /*  if(isService){
        	   s->dstart(10000,6);
        	   return;
           }*/

       }
       if(isService || autoRun) s->start();
}

void dgserver::stop(){  if(s) s->stop();}
void dgserver::pause(){ if(s) s->stop();}
void dgserver::resume(){if(s) s->start();}
//void dgserver::processCommand(int code){qDebug()<<code;}


void dgserver::initDG(){


	   	XSD::appPath=application()->applicationDirPath();
	   	application()->addLibraryPath(XSD::appPath);

	   	#if defined(Q_OS_WIN)
	   	      SYSTEM::LogFileName =  application()->applicationFilePath().replace(".exe", ".log",Qt::CaseInsensitive);
	   	#else
	   	      SYSTEM::LogFileName =  application()->applicationFilePath().append(".log");
	   	#endif

	   	LogF=new logFile(SYSTEM::LogFileName);  //если хотим сообщения направлять в лог!!!

	   	#if defined(Q_OS_WIN)
	   		  LogF->openLog();
	   	#else
	   		  if(SYSTEM::Debug && !isService){} //отладка в консоль
	   		  else LogF->openLog();
	   	#endif


}

int dgserver::testApp(){
	if(application()->sendMessage("Wake up dgserver!",1000)) return 2;
	return 0;
}


