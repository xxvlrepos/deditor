/*
 * ExeThread.cpp
 *
 *  Created on: 02 окт. 2014 г.
 *      Author: Usach
 */

#include "ExeThread.h"



ExeThread::ExeThread(tape* tp, QObject *parent): QThread(parent){
 	ttp=tp;
 }

ExeThread::~ExeThread() {
	halt();
//	qWarning()<<"del tread";
}



void ExeThread::run(){

 	QSignalMapper signalMapper;

 	exe=new execut(0);
 	exe->setTape(ttp);

 	connect(exe,  SIGNAL(finish(int)),this,SLOT(exit()));
  	connect(this, SIGNAL(stopEXE()),exe,SLOT(stop()));
 	connect(exe,SIGNAL(rezult(command*)),this,SLOT(headRez(command*)));
 	exe->Start();

   	exec();
}

void ExeThread::headRez(command* cmd){

//	execut* exe = qobject_cast<execut*>(sender());

	if(cmd->Type==IF){
		if(cmd->getRez()==BAD)         qWarning()<<"Error bookmarks";
		else if(cmd->getRez()==ERRCON) qWarning()<<"Osc: Exceeded number of errors!";
	}
	if(cmd->Type==DOWNLOAD){
		dDOWNLOAD* dd=(dDOWNLOAD*)cmd;
		switch(cmd->getRez()){
			case ERRCON:
				 qWarning()<<dd->shortName+dd->afterName+" Exceeded number of communication errors!";
				 break;
			case OK:
				emit fileLoaded(dd->fileName);
				break;
			case BAD:
				qWarning()<<dd->shortName+dd->afterName+" Data clients errors!";
			    break;
		}
	}

	exeRez(cmd);

}

void ExeThread::exeRez(command* cmd){};



void ExeThread::halt(){
	  //  qWarning()<<"halt!";
    	emit stopEXE();
}

void ExeThread::exit(){

	//if(execut* exe = qobject_cast<execut*>(sender())){
		delete(exe);
		exe=NULL;
	//	qWarning()<<"del exe";
	//}
		quit();
}


