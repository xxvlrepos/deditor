/*
 * Sheduler.cpp
 *
 *  Created on: 09 окт. 2014 г.
 *      Author: Usach
 */

#include "Sheduler.h"


//******************************************************************************
JobT::JobT(){
	testDT.toLocalTime();
	testDT.setTime_t(0);
}


bool JobT::test(QDateTime cDT){

	bool rez=false;
	if(min.size()==0 || hour.size()==0) return rez;
	if(testDT<=cDT){
		if(testDT.date().year()!=1970) rez=true;
		testDT=newDT(cDT);
	}
	return rez;
}

QDateTime JobT::newDT(QDateTime curDT){

	QTime time;
	for(int i=0;i<hour.size();i++){
		for(int j=0;j<min.size();j++){
			time.setHMS(hour[i],min[j],0);
			if(time>curDT.time()) {
				curDT.setTime(time);
				return curDT;
			}
		}
	}
	time.setHMS(hour[0],min[0],0);
	curDT.setTime(time);
	return curDT.addDays(1);
}


bool JobT::addShed(QString m, QString h){

	bool ok1,ok2;
	min.clear();
	hour.clear();

	min.append(parser(m,60,&ok1));
	hour.append(parser(h,24,&ok2));
	//for(int i=0;i<min.size();i++) qDebug()<<(int)min.at(i);
	//for(int i=0;i<hour.size();i++) qDebug()<<(int)hour.at(i);
	if(!ok1 || !ok2){
		min.clear();
		hour.clear();
		return false;
	}
    return true;
}


QList<unsigned char> JobT::parser(QString d,int max,bool *rez){

	  *rez=false;
	  QList<unsigned char> lrez;
	  char n1,n2,n3,ns;
	  bool ok1,ok2,ok3;
	  QVector<bool> vd(max,false);

      QStringList l=d.split(',',QString::SkipEmptyParts);
      for(int i=0;i<l.size();i++){
    	  l[i]=l.at(i).trimmed();
    	  QStringList ll=l[i].split('-');

    	  if(ll.size()>2) return lrez;

    	  if(ll.size()==1){
    		  ll[0]=ll[0].trimmed();

    		  if(l.size()==1 && ll[0].size()>2 && ll[0].left(2)=="*/"){
    			  n1=ll[0].mid(2).toUShort(&ok1);
    			  if(ok1 && n1<=max){
    				  ns=0;
    				  while(ns<max){
    					  vd[ns]=true;
    					  ns+=n1;
    				  }
    				  break;
    			  }else          return lrez;
    		  }

    		  if(l.size()==1 && ll[0]=="*"){
    			  for(int n=0;n<max;n++)   vd[n]=true;
    		  }else{
				  n1=ll[0].toUShort(&ok1);
				  if(ok1){
					  if(n1<max) vd[n1]=true;
					  if(n1==max) vd[0]=true;
					  if(n1>max) return lrez;
				  }else          return lrez;
    		  }
    	  }

    	  if(ll.size()==2){  //1-24  и 1-24/2
    		  ll[0]=ll[0].trimmed();
    		  ll[1]=ll[1].trimmed();

    		  QStringList lll=ll[1].split('/');

    		  if(lll.size()==1){              //1-24
    			  n1=ll[0].toUShort(&ok1);
    			  n2=ll[1].toUShort(&ok2);
    			  n3=1; ok3=true;
    		  }else if(lll.size()==2){          //1-24/2
    			  lll[0]=lll[0].trimmed();
    			  lll[1]=lll[1].trimmed();

    			  n1=ll[0].toUShort(&ok1);
    			  n2=lll[0].toUShort(&ok2);
    			  n3=lll[1].toUShort(&ok3);
    		  }else return lrez;


    		  if(ok1 && ok2 && ok3){
    			  if(n2<n1)  return lrez;
    			  if(n2>max) return lrez;
    			  if(n3>max || n3<1) return lrez;
    			  if(n2==max) {vd[0]=true; n2=max-1;}
    			  ns=n1;
    			  while(ns<=n2){
    			  	 if(ns<max)  vd[ns]=true;
    			  	 if(ns==max) vd[0] =true;
    			  	  ns+=n3;
    			  }

    			  for(int n=n1;n<=n2;n++)   vd[n]=true;
    		  }else return lrez;
    	  }

      }


      for(int i=0;i<vd.size();i++){
    	  if(vd[i]) lrez<<(unsigned char)i;
      }
      *rez=true;

      return lrez;
}

//********************************************************************************


Sheduler::Sheduler(QObject *parent): QObject(parent){
	// TODO Auto-generated constructor stub

	timer.setSingleShot(false);
	curDTime.setTimeSpec(Qt::LocalTime);
	connect(&timer, SIGNAL(timeout()), this,  SLOT(test()));
}

Sheduler::~Sheduler() {

	clear();
	// TODO Auto-generated destructor stub
}

bool Sheduler::append(QString m, QString h){
	bool rez=false;
	JobT Job;
	rez=Job.addShed(m,h);
	if(rez) shedList<<Job;
	return rez;
}

void Sheduler::start(){
	timer.start(timeInt);
	qDebug()<<"Shedule start:" <<shedList.size();
}

void Sheduler::clear(){
	timer.stop();
	shedList.clear();
}

void Sheduler::test(){

	curDTime =QDateTime::currentDateTime();
	for(int i=0; i<shedList.size(); i++){
			if(shedList[i].test(curDTime)) emit startJob(i);
	}
}

