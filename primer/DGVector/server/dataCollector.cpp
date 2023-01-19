/*
 * dataCollector.cpp
 *
 *  Created on: 30 окт. 2014 г.
 *      Author: Usach
 */

#include "dataCollector.h"
#include "dataServer.h"

dataCollector::dataCollector(QString cName,mMap* md , QDomDocument doc,QString& type, QObject *parent) :  QObject(parent) {

	modbusDat=md;
	xmldoc=doc;
//	qDebug()<<type;
	if(type=="QMYSQL"){
		tableSER= new TableSER();
		tableACT= new TableACT();
		tableTD=  new TableTD();
		tableHD=  new TableHD();
		tableSTD= new TableSTD();
		tableDITD=new TableDITD();
		tableOPT= new TableOPT();
		updTriger= new UpdTrigerM();
		clearProc=new  ClearProcM();

	}else if(type=="QPSQL"){
		tableSER= new TableSER_P();
		tableACT= new TableACT_P();
		tableTD=  new TableTD_P();
		tableHD=  new TableHD_P();
		tableSTD= new TableSTD_P();
		tableDITD=new TableDITD_P();
		tableOPT= new TableOPT_P();
		updTriger=new UpdTrigerP();
		clearProc=new ClearProcP();
	}


/*	nameACT="status";
	nameTD="opdat";
	nameHD="hidat";
	nameSTD="stdat";
	nameDITD="dist";
	nameOPT="tegsname";
*/

	deepHis=0;//минут
	nameCon=cName;
	thread=NULL;
}

dataCollector::~dataCollector() {
	stop();
	deleteThred();
	delete(tableSER);
	delete(tableACT);
	delete(tableTD);
	delete(tableHD);
	delete(tableSTD);
	delete(tableDITD);
	delete(tableOPT);
	delete(updTriger);
	delete(clearProc);

}

bool dataCollector::dbConnect(QDomElement sElem){


	if(xmldoc.isNull()) return false;
	rootNode=xmldoc.documentElement();
	bool ok;
	parConn=sElem;
	deepHis=sElem.attribute("histdata","0").toInt(&ok);
	if(deepHis==0) return false;
	return true;
}

void dataCollector::iniTable(){
	if(!QSqlDatabase::contains(nameCon)) return;
	QSqlDatabase db(QSqlDatabase::database(nameCon));
	if(!db.open()) return;
	QString str;



	QSqlQuery qd1(tableTD->drop(),db);

	// Создаем таблицу оперативных данных
	QSqlQuery q1(db);
	if (q1.exec(tableTD->greate())) qDebug()<<"Create database table '"+tableTD->Name+"'";

	// Создаем таблицу истории
	/*if(deepHis==0)*/ QSqlQuery qd2(tableHD->drop(),db);
	QSqlQuery q2(db);
	if (q2.exec(tableHD->greate())) qDebug()<<"Create database table '"+tableHD->Name+"'";


	//создаем тригер
	str=updTriger->greateF();
	if(str!="")	QSqlQuery qtf(str,db); //тригерная функция - если есть
	QSqlQuery qtt(db);
//	qDebug()<<updTriger->greate();
	if (qtt.exec(updTriger->greate())) qDebug()<<"Create trigger";


	//создаем процедуру очистки
	QSqlQuery qdrop(clearProc->drop(),db);
	QSqlQuery qcrea(db);
	if (qcrea.exec(clearProc->greate())) qDebug()<<"Create procedure '"+clearProc->Name+"'";
	else                                 qDebug()<<"Error create procedure '"+clearProc->Name+"'";

	// Создаем таблицу  статических данных
	QSqlQuery qd3(tableSTD->drop(),db);
	QSqlQuery q3(db);
	if (q3.exec(tableSTD->greate())) qDebug()<<"Create database table '"+tableSTD->Name+"'";

	// Создаем словарь типов каналов данных
	QSqlQuery qd4(tableDITD->drop(),db);
	QSqlQuery q4(db);
	if (q4.exec(tableDITD->greate())) qDebug()<<"Create database table '"+tableDITD->Name+"'";


	// Создаем словарь описатия данных
	QSqlQuery qd5(tableOPT->drop(),db);
	QSqlQuery q5(db);
	if (q5.exec(tableOPT->greate())) qDebug()<<"Create database table '"+tableOPT->Name+"'";

	str=   "INSERT INTO "+tableOPT->Name+" VALUES ";
	QString nm;
	QDomNodeList nl=rootNode.childNodes();

	bool ok;
	uint id;
	for (int i=0;i<nl.size();i++){
		QDomNode cn=nl.at(i);
		nm=cn.nodeName().toUpper();
		QDomNodeList nle=cn.childNodes();
		QList<uint> listId;
		for(int j=0;j<nle.size();j++){
			QDomNode cne=nle.at(j);
			QDomNamedNodeMap amap=cne.attributes();
			id=amap.namedItem("id").nodeValue().toUInt(&ok,16);
			if(id==0) continue;
			if(!listId.contains(id)){
			   listId<<id;
			   tableOPT->setValue(0,"'"+nm+"'");
			   tableOPT->setValue(1,QString::number(id,10));
			   tableOPT->setValue(2,amap.namedItem("divider").nodeValue());
			   tableOPT->setValue(3,"'"+amap.namedItem("name").nodeValue()+"'");
			   tableOPT->setValue(4,"'"+amap.namedItem("dimension").nodeValue()+"'");
			   tableOPT->setValue(5,"'"+amap.namedItem("title").nodeValue()+"'");
			   str+=tableOPT->getVList()+",";
			}else{
				qDebug()<<tr("Error! Element with the %1:%2 in already the dictionary").arg(nm).arg(QString::number(id,16));
			}
		}

	}
	str[str.size()-1]=';';
	QSqlQuery qi5(str,db);


	QSqlQuery qd6 (tableACT->drop(),db);
	// Создаем таблицу состояний
	QSqlQuery q6(db);
	if (q6.exec(tableACT->greate())) qDebug()<<"Create database table '"+tableACT->Name+"'";

	QSqlQuery qd7 (tableSER->drop(),db);
	// Создаем таблицу состояний сервера
	QSqlQuery q7(db);
	if (q7.exec(tableSER->greate())) qDebug()<<"Create database table '"+tableSER->Name+"'";
	tableSER->setValue(0,"1");
	tableSER->setValue(1,"now()");
	QSqlQuery qi7(tableSER->insert(),db);



	QString ma;
	str=   "INSERT INTO "+tableACT->Name+" VALUES ";
	mMap::iterator it=modbusDat->begin();

	for(;it!=modbusDat->end();it++){
		ma.setNum(it.key(),10);
		tableACT->setValue(0,ma);
		tableACT->setValue(1,"0");
		tableACT->setValue(2,"now()");
		tableACT->setValue(3,"'"+it.value()->tipCon+"'");
		if(it.value()->ip=="") tableACT->setValue(4,"'"+it.value()->comaddr+"'");
		else				   tableACT->setValue(4,"'"+it.value()->ip+"'");
		tableACT->setValue(5,"'"+it.value()->alias+"'");
		actData[it.key()]=false;
		str+=tableACT->getVList()+",";
	}
	str[str.size()-1]=';';
	QSqlQuery qi6(str,db);
}

void dataCollector::closeACT(){
	if(!QSqlDatabase::contains(nameCon)) return;
	QSqlDatabase db(QSqlDatabase::database(nameCon));
	if(!db.open()) return;
	QString str("UPDATE "+tableACT->Name+" SET state=0, ctime=now();");
	QSqlQuery q(str,db);

}

void dataCollector::genDataList(int modbus,const QStringList& ls){


	mutex.lock();
	if(memDat.contains(modbus)) memDat.remove(modbus);
	mdCan* curCan;
	mdCan  staticCan;

	QHash<unsigned char,QDomNode> mapNode;
	QDomElement curE;
	QDomNodeList dList;
	unsigned short sm,id;
	QString txt,txi;
	bool ok;
	int l,a;
	QList<unsigned short> readDR;
	for(int i=0;i<ls.size();i++){
		l=ls[i].right(2).toInt(&ok,16);
		a=ls[i].left(4).toInt(&ok,16)+0x1000;
		for(int j=0;j<l;j++){
			readDR.append(a);
			a++;
		}
	}


	QDomNodeList list=rootNode.childNodes();
	for (int i=0;i<list.size();i++)	mapNode[list.at(i).nodeName()[0].toUpper().cell()]=list.at(i);

	MArray*  mdbD=modbusDat->value(modbus);
	infoADC& inf=mdbD->inf;
	memDat[modbus] = new mdCan();
	curCan=memDat[modbus];
	QString tx;

	   for(int i=0; i<inf.adc.size();i++){
		   uchar t=0;
		   if(inf.adc[i]!=0){
			   t=tx.setNum(inf.adc[i],16).toUpper()[0].cell();
			   if(t=='2')      t='M';
			   else if(t=='3') t='I';
			   curE=mapNode[t].toElement();
		   }
		   else continue;

		   sm=curE.attribute("s"+txt.setNum(i+1)).toUShort(&ok,16);
		   dList= curE.childNodes();
		   for(int j=0;j<dList.size();j++){
			  if(dList.at(j).attributes().namedItem("read").nodeValue()!="0"){
				 id=dList.at(j).attributes().namedItem("id").nodeValue().toUShort(&ok,16);
				 mdCan::iterator it;
				 if(readDR.contains(sm+id)) it= curCan->insert(sm+id,mdbD->getMData(sm+id));
				 else                       it= staticCan.insert(sm+id,0);
				 it.value().num=i+1;
				 it.value().kType=t;
				 it.value().id=id;
			  }
		   }

		}

	   //тахометры
	   curE=mapNode['T'].toElement();
	   dList= curE.childNodes();
	   for(int i=0; i<inf.tah.size();i++){
			if(inf.tah[i]){
				sm=curE.attribute("s"+txt.setNum(i+1)).toUShort(&ok,16);
				for(int j=0;j<dList.size();j++){
				  if(dList.at(j).attributes().namedItem("read").nodeValue()!="0"){
					 id=dList.at(j).attributes().namedItem("id").nodeValue().toUShort(&ok,16);
					 mdCan::iterator it;
					 if(readDR.contains(sm+id)) it= curCan->insert(sm+id,mdbD->getMData(sm+id));
					 else                       it= staticCan.insert(sm+id,0);
					 it.value().num=i+1;
					 it.value().kType='T';
					 it.value().id=id;
					 //it.value().unsig=(bool)dList.at(j).toElement().attribute("unsigned","0").toUShort(&ok);

				  }
			   }
			}
		}

		//виртуальные каналы
		for(int i=0; i<inf.vir.size();i++){
		   if(inf.vir[i]!=0){
			   tx=tx.setNum(inf.vir[i],16).toUpper();
			   curE=mapNode[tx[0].cell()].toElement();
		   }
		   else continue;
		   sm=curE.attribute("s"+txt.setNum(i+1)).toUShort(&ok,16);
		   dList= curE.childNodes();
		   for(int j=0;j<dList.size();j++){
			   QDomNamedNodeMap map=dList.at(j).attributes();
			  if(dList.at(j).attributes().namedItem("read").nodeValue()!="0"){
				 id=dList.at(j).attributes().namedItem("id").nodeValue().toUShort(&ok,16);
				 mdCan::iterator it;
				 if(readDR.contains(sm+id)) it= curCan->insert(sm+id,mdbD->getMData(sm+id));
				 else                       it= staticCan.insert(sm+id,0);
				 it.value().num=i+1;
				 it.value().kType=tx[0].cell();
				 it.value().id=id;
			  }
		   }
		}

	   //термометры
	   curE=mapNode['N'].toElement();
	   dList= curE.childNodes();
	   for(int i=0; i<inf.term.size();i++){
			if(inf.term[i]){
				sm=curE.attribute("s"+txt.setNum(i+1)).toUShort(&ok,16);
				for(int j=0;j<dList.size();j++){
				  if(dList.at(j).attributes().namedItem("read").nodeValue()!="0"){
					 id=dList.at(j).attributes().namedItem("id").nodeValue().toUShort(&ok,16);
					 mdCan::iterator it;
					 if(readDR.contains(sm+id)) it= curCan->insert(sm+id,mdbD->getMData(sm+id));
					 else                       it= staticCan.insert(sm+id,0);
					 it.value().num=i+1;
					 it.value().kType='N';
					 it.value().id=id;
				  }
			   }
			}
		}

//		if(curCan->size()==0) 	memDat.remove(modbus);



		QStringList QueryList;
		QString m,ad,v;
		m.setNum(modbus,10);


		QueryList<<"DELETE FROM "+tableTD->Name+" WHERE modbus="+m+";";
		QueryList<<"DELETE FROM "+tableDITD->Name+" WHERE modbus="+m+";";



		QString  sqz("INSERT INTO "+tableTD->Name+" VALUES ");
		QString  sqd("INSERT INTO "+tableDITD->Name+" VALUES ");

		if(curCan->size()>0 && 	deepHis>0){
			mdCan::iterator it=	curCan->begin();
			for(;it!=curCan->end();it++){
				ad.setNum(it.key(),10);
				//if(it.value().unsig) v.setNum((unsigned short)it.value().value,10);
				//else
				v.setNum(it.value().value,10);
				tableTD->setValue(0,m);
				tableTD->setValue(1,ad);
				tableTD->setValue(2,v);
				tableTD->setValue(3,"now()");
				sqz+=tableTD->getVList()+",";

				tableDITD->setValue(0,m);
				tableDITD->setValue(1,ad);
				tableDITD->setValue(2,txt.setNum(it.value().num));
				tableDITD->setValue(3,"'"+QString(it.value().kType) +"'");
				tableDITD->setValue(4,txi.setNum(it.value().id));
				sqd+=tableDITD->getVList()+",";
			}
		}
		if(staticCan.size()>0){
			mdCan::iterator it=	staticCan.begin();
			for(;it!=staticCan.end();it++){
				ad.setNum(it.key(),10);
				tableDITD->setValue(0,m);
				tableDITD->setValue(1,ad);
				tableDITD->setValue(2,txt.setNum(it.value().num));
				tableDITD->setValue(3,"'"+QString(it.value().kType) +"'");
				tableDITD->setValue(4,txi.setNum(it.value().id));
				sqd+=tableDITD->getVList()+",";
			}
		}

		//qDebug()<<sqz;

		if(sqz[sqz.size()-1]==',') 	sqz[sqz.size()-1]=';';
		if(sqd[sqd.size()-1]==',')  sqd[sqd.size()-1]=';';
		QueryList<<sqz<<sqd;
		addInQue(QueryList);

	mutex.unlock();
}

void dataCollector::testData(unsigned char m,QList<sendD>& dl){


	mutex.lock();

	if(memDat.contains(m) && modbusDat->contains(m) ){

		MArray* mdC=modbusDat->value(m);
		mdCan*  curL=memDat[m];
		short cd;
		mdCan::iterator it=curL->begin();
		for(;it!=curL->end();it++){
			cd=mdC->getMData(it.key());
			if(cd!=it.value().value){
				it.value().value=cd;
				dl.append(sendD(m,it.key(),cd));
			}
		}

	}
	mutex.unlock();
}

QList<dataCollector::sendD> dataCollector::getListData(){
		QList<sendD> send;
		mMap::iterator it=modbusDat->begin();
		for(;it!=modbusDat->end();it++){
			if(it.value()->dstatus) testData(it.key(),send);
		}
		return send;
}

QList<dataCollector::sendSTAT> dataCollector::getListStat(){
	QList<sendSTAT> send;
		mMap::iterator it=modbusDat->begin();
		for(;it!=modbusDat->end();it++){
			  	if(it.value()->dstatus!=actData[it.key()]){
					send.append(sendSTAT(it.key(),it.value()->dstatus));
					actData[it.key()]=it.value()->dstatus;
				}
		}
	return send;

}

void dataCollector::sendDbDat(){

	if(QSqlDatabase::contains(nameCon)){
		QSqlDatabase db(QSqlDatabase::database(nameCon));
		if(db.open()){
			    db.transaction();
				QList<sendD> send(getListData());

				if(send.size()>0){
					QList<sendD>::const_iterator it=send.begin();
					for(;it!=send.end();it++){

						QString zp("UPDATE "+tableTD->Name+" SET ");
						tableTD->setValue(0,it->sm);
						tableTD->setValue(1,it->sa);
						tableTD->setValue(2,it->sv);
						tableTD->setValue(3,"now()");
						zp+=tableTD->getColumEQ(2)+", "+tableTD->getColumEQ(3)+" WHERE ";
						zp+=tableTD->getColumEQ(0)+" AND "+tableTD->getColumEQ(1)+";";
						//qDebug()<<zp;
						QSqlQuery qr(zp,db);
					}
				}
				QSqlQuery qr(tableSER->update(),db);

				QList<sendSTAT> sendS(getListStat());
				if(sendS.size()>0){
					QList<sendSTAT>::const_iterator itS=sendS.begin();
					for(;itS!=sendS.end();itS++){
						QString zpS("UPDATE "+tableACT->Name+" SET ");
						tableACT->setValue(0,itS->sm);
						tableACT->setValue(1,itS->ss);
						tableACT->setValue(2,"now()");
						zpS+=tableACT->getColumEQ(1)+", "+tableACT->getColumEQ(2)+" WHERE ";
						zpS+=tableACT->getColumEQ(0)+";";
						QSqlQuery qrS(zpS,db);
					}
				}

				db.commit();
		}
	}
}

void dataCollector::createStaticDT(int m,const QStringList& ls){

	int l,a;
	bool ok;
	MArray* mdC=modbusDat->value(m);
	QList<sendD> send;
	for(int i=0;i<ls.size();i++){
		l=ls[i].right(2).toInt(&ok,16);
		a=ls[i].left(4).toInt(&ok,16)+0x1000;
		for(int j=0;j<l;j++){
			send.append(sendD((unsigned char)m,a,mdC->getMData(a)));
			a++;
		}
	}


	QStringList queryList;
	QString ms;
	ms.setNum(m,10);

	queryList<<"DELETE FROM "+tableSTD->Name+" WHERE modbus="+ms+";";

	if(send.size()>0){
		QString  sqz("INSERT INTO "+tableSTD->Name+" VALUES ");
		QList<sendD>::const_iterator it=send.begin();
		for(;it!=send.end();it++){
			tableSTD->setValue(0,it->sm);
			tableSTD->setValue(1,it->sa);
			tableSTD->setValue(2,it->sv);
			sqz+=tableSTD->getVList()+",";
		}
		sqz[sqz.size()-1]=';';
		queryList<<sqz;
	}
	addInQue(queryList);
}

void dataCollector::clearHistT(){
	if(QSqlDatabase::contains(nameCon)){
			QSqlDatabase db(QSqlDatabase::database(nameCon));
			if(db.open()){
				QSqlQuery qc(clearProc->call(QString::number(deepHis)),db);
			}
	}
}



void dataCollector::start(){

	if(deepHis==0) return;
	thread=new dbThread(this);
	thread->moveToThread(thread);
	connect(thread, SIGNAL(finished()), this, SLOT(deleteThred()));
	connect(this, SIGNAL(stopAll()),  thread, SLOT(halt()));
	thread->start();
}

void dataCollector::stop(){
    emit stopAll();
}

void dataCollector::deleteThred(){
	//dbThread* thread = qobject_cast<dbThread*>(sender());
	if(thread){
		thread->wait();
		if(thread){	delete(thread);	thread=NULL;}
	}
  }

void dataCollector::addInQue(const QStringList& listQ){
	mutexQue.lock();
	 	 queQuery.append(listQ);
	mutexQue.unlock();
}

void dataCollector::sendQury(){
	QString qtxt;
	int sz;
	while(1){
		mutexQue.lock();
	 	sz=queQuery.size();
	 	if(sz>0) qtxt=queQuery.dequeue();
	 	mutexQue.unlock();
	 	if(sz==0) break;
	 	if(QSqlDatabase::contains(nameCon)){
	 		QSqlDatabase db(QSqlDatabase::database(nameCon));
	 		if(db.open())	QSqlQuery qu(qtxt,db);
	 	}
	}

}

void dataCollector::genStatic(int m){
	createStaticDT(m,dataServer::genStatReg(&modbusDat->value(m)->inf));
}

void dataCollector::genDinamic(int m){
	genDataList(m,dataServer::genDinReg(&modbusDat->value(m)->inf));
}

