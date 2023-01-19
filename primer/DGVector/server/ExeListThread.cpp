/*
 * ExeListThread.cpp
 *
 *  Created on: 02 окт. 2014 г.
 *      Author: Usach
 */

#include "ExeListThread.h"
#include "dataServer.h"

uchar  ExeListThread::debugN=0;

ExeListThread::ExeListThread(const QList<tape*>& ltp,mMap* m, QObject *parent): QThread(parent){
 	lttp=ltp;
 	map=m;
 	maxS=0;
 	runS=true;
 }

//ExeListThread::~ExeListThread() {
	// TODO Auto-generated destructor stub
//}

void ExeListThread::restartExe(int n,tape* t){
 	if(n<lexe.size()){
 		lexe[n]->setTape(t);
 		emit restartN(n);
 	}
 }

void ExeListThread::restartExe(int n){
 	if(n<lexe.size()){
 		emit restartN(n);
 	}
}


void ExeListThread::run(){

 	signalMapper = new QSignalMapper();
 	unsigned char m;
 	uint mdeb=0;
 	execut* exeMaster=NULL;
 	for(int i=0;i<lttp.size();i++){
   		if(debugN==lttp[i]->marker){
   			mdeb=debugN;
   			break;
   		}
   	}

	for(int i=0;i<lttp.size();i++){
		  execut* exe=new execut(0);
		  exe->setTape(lttp[i]);
		  lexe.append(exe);
		  m=lttp[i]->marker;

		  if(m>0){
			  exe->enDebug(map->value(m)->Debug ? m : 0);
			  exe->enTdebug(map->value(m)->Tdebug);
			  map->value(m)->exe=exe;
			  map->value(m)->dstatus=false;
		  }

		  if(lttp[i]->first()->getType()==CONNECT){
			  if(((dCONNECT*)lttp[i]->first())->cType!=cEXT){
				  exeMaster=exe;
				  exeMaster->enDebug(mdeb);
			  }else{
				  if(exeMaster==NULL) qWarning("Error! ExeMaster is NULL!");
				  lexe[i]->extExe=exeMaster;
			  }
		  }

		  connect(exe, SIGNAL(finish(int)), signalMapper, SLOT(map()));
		  connect(exe,SIGNAL(rezult(command*)),this,SLOT(headRez(command*)));
		  connect(exe,SIGNAL(messageError(QString)),this,SLOT(mError(QString)));
		  connect(exe,SIGNAL(timeCycle(uchar,int,int)),this,SLOT(statCon(uchar,int,int)));
		  signalMapper->setMapping(exe,lexe.size()-1);
		  if(m>0) map->value(m)->status=MArray::eInfo;
	}

 	connect(signalMapper, SIGNAL(mapped(int)),this,SLOT(finish(int)));
 	connect(this,SIGNAL(restartN(int)),this,SLOT(startN(int)));
 	connect(this,SIGNAL(halt()),this,SLOT(haltALL()));

 	for(int i=0;i<lexe.size();i++){
 		//if(exeMaster!=NULL && lexe[i]!=exeMaster)	lexe[i]->extExe=exeMaster;
 		lexe[i]->Start();
 	}
   	exec();
  }


void ExeListThread::headRez(command* cmd){

	execut* exe = qobject_cast<execut*>(sender());

	if(cmd->getRez()==ERRCON){
		//if(exe->getTape()->getMaxRepeat()>0) emit message("Connection failures is exceeded!",1);
	}

	if(cmd->Type==IF){
		dIF* di=(dIF*)cmd;
		if(di->getRez()==OK){
			emit update(di->getAFCmd(),di->getData());
			//mMap::mdbMap->updateData(di->getAFCmd(),di->getData());
			bool ok;
			QString txt=di->stOK;
			int m=txt.mid(1).toInt(&ok,10);
			if(ok && m>0){
				 MArray* md=map->value(m);
				 if(md->dstatus) return;
				 if(txt.left(1)=="s"){
					 emit finStat(m);
					 return;
				 }
				 if(txt.left(1)=="d"){
					md->dstatus=true;
					emit finDin(m);
					return;
				 }
			}

		}
	}
}

void ExeListThread::stop(){
	disconnect(signalMapper, SIGNAL(mapped(int)),this,SLOT(finish(int)));
	emit halt();
}

void ExeListThread::haltALL(){
	int m=0;

//	if(lexe.size()>0) qDebug()<<"halt All"<<lexe.size()<<"exe";
	for(int i=0;i<lexe.size();i++){
		execut *exe=lexe.at(i);
		if(exe->getStatus()) {
			disconnect(exe,SIGNAL(rezult(command*)),this,SLOT(headRez(command*)));
			m=exe->getTape()->marker;
		//	qDebug()<<"stopped exe"<<m<<exe->getTransport();
			exe->Stop();
		}
	}
 	while(lexe.size()>0){
 		execut *exe=lexe.takeFirst();//начинаем с родителей
 		m=exe->getTape()->marker;
 		if(m) map->value(m)->exe=NULL;
 		delete(exe);
 	}
 	lttp.clear();
 	quit();
}



void ExeListThread::finish(int n){
	//qDebug()<<"finish exes"<<n<<":"<<lexe[n]->getTape()->marker;
 	emit finished(lexe[n]->getTape()->marker);
 }

void ExeListThread::startN(int n){
		lexe[n]->Start();
}


void ExeListThread::statCon(uchar id,int t,int tc){
	if(!id || !runS) return;
	if(!statH.contains(id)) statH[id]=0;
	else runS=(++statH[id]<9);
	if(maxS<t) maxS=t;
	if(id==dataServer::NumDebugCycle && statH.contains(id)){
		 emit message(QString::number(id)+":cycle time:"+
				 QString::number(t)+"/"+
				 QString::number(maxS)+"/"+QString::number(tc));
		 maxS=0;
	}

}
