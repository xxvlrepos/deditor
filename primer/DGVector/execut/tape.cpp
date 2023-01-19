/*
 * tape.cpp
 *
 *  Created on: 13.07.2012
 *      Author: Usach
 */

#include "tape.h"





void tape::clear(){

	 for (int i=0; i<size(); i++)  delete(value(i));
	 QList<command*>::clear();
}

command*  tape::append(commandType Type,QString label){

	switch(Type){
		case IF: 		QList<command*>::append(new dIF); 		break;
		case FOR:		QList<command*>::append(new dFOR);		break;
		case DOWNLOAD:  QList<command*>::append(new dDOWNLOAD);	break;
		case MULTICAST: QList<command*>::append(new dMULTICAST);break;
		case CONNECT:   QList<command*>::append(new dCONNECT);	break;
		case DISCONNECT:QList<command*>::append(new dDISCONNECT);break;
		case SETTIME:   QList<command*>::append(new dSETTIME);	break;
		case BROADCAST:	QList<command*>::append(new dBROADCAST);break;
		case NOOP:  	QList<command*>::append(new dNOOP);     break;

	}
	last()->setLabel(label);
	last()->setOK(size());
	last()->setBAD(size());
	return last();
}

bool tape::test(bool debug){
	command* it;
	QString txt;
	bool r=true;

	int adrOK,adrBAD;
	if(debug) qDebug()<<"mark:"<<marker;
	for(int i=0;i<size();i++){
		it=value(i);
		if(it->goOK_L.size()>0){
			adrOK=find(it->goOK_L);
			if(adrOK>=0 && adrOK<size()) it->setOK(adrOK);
			else{
				r=false;
				qDebug() <<"str:"+txt.setNum(i)+" Label '"+it->goOK_L+"' not found";
			}
		}
		if(it->goBAD_L.size()>0){
			adrBAD=find(it->goBAD_L);
			if(adrBAD>=0 && adrBAD<size()) it->setBAD(adrBAD);
			else{
				r=false;
				qDebug() <<"str:"+txt.setNum(i)+" Label '"+it->goBAD_L+"' not found";
			}
		}
		if(it->getOK()>size() || it->getBAD()>size()){
			    r=false;
				qDebug() <<"str:"+txt.setNum(i)+" GOTO addr. bad";
		}
		//debug=1;
		if(debug){
			QString str;
			if(i<10) str="str: "+txt.setNum(i)+" ";
			else     str="str:" +txt.setNum(i)+" ";
			switch(it->getType()){
					case IF: 		str+="IF        ";	break;
					case FOR:		str+="FOR       ";	break;
					case DOWNLOAD:  str+="DOWNLOAD  ";	break;
					case MULTICAST: str+="MULTICAST ";	break;
					case CONNECT:   str+="CONNECT   ";	break;
					case DISCONNECT:str+="DISCONNECT";	break;
					case SETTIME:   str+="SETTIME   ";	break;
					case BROADCAST: str+="BROADCAST ";	break;
					case NOOP:      str+="NOOP      ";	break;
			}
			str+=" OK="+txt.setNum(it->getOK());
			str+=" BAD="+txt.setNum(it->getBAD());
			if(it->getLabel().size()>0) str+=" l:"+it->getLabel().toUpper();
			if(it->goOK_L.size()>0) 	str+=" OK:"+it->goOK_L.toUpper();
			if(it->goBAD_L.size()>0) 	str+=" BAD:"+it->goBAD_L.toUpper();
			qDebug()<<str;
		}
	}
	return r;
}

int tape::find(QString l){
	for(int i=0;i<size();i++){
		if(value(i)->getLabel()==l.toUpper()) return i;
	}
	return -1;
}
