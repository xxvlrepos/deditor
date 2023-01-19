/*
 * sdiagCollector.cpp
 *
 *  Created on: 28 февр. 2017 г.
 *      Author: Usach
 */

#include "sdiagCollector.h"
#include "dataServer.h"


const double sdiagCollector::TwoPi = 6.28318530717958647692;
const QVector<QString> sdiagCollector::frList=IniQVector<QString>()<<"125"<<"250"<<"500"<<"1000"<<"2000"<<"4000"<<"8000";
const QVector<QString> sdiagCollector::nameRMS=IniQVector<QString>()<<"Ae"<<"Ve"<<"Se";
const QVector<QString> sdiagCollector::sizeRMS=IniQVector<QString>()<<tr("m/c^2")<<tr("mm/c")<<tr("mkm");
const QVector<unsigned short> sdiagCollector::bazeList=IniQVector<unsigned short>()<<0x1000<<0x1100<<0x1200<<0x1300<<0x1700<<0x1800;
const QVector<unsigned short> sdiagCollector::bazeSpList=IniQVector<unsigned short>()<<0x4000<<0x4100<<0x4200<<0x4300<<0x4700<<0x4800;


sdiagCollector::sdiagCollector(mMap* m,QDomElement dbPar,QString& type, QObject *parent): QObject(parent) {

	map=m;
	run=false;
	dcon=NULL;
	shedDiag=NULL;
	toDB=false;
	if(!dbPar.isNull()){
		toDB=true;
		parConn=dbPar;
		cName="sdiagToDB";
		if(type=="QMYSQL"){
			tableSData=new TableSData_M();
			tableDist=new TableDist_M();
			tableName=new TableName_M();
		}else if(type=="QPSQL"){
			tableSData=new TableSData_P();
			tableDist=new TableDist_P();
			tableName=new TableName_P();
		}
	}
//	eTred=NULL;

	shedDiag=new Sheduler(this);

	if(unsheduled){
		   timer.setSingleShot(false);
		   connect(&timer, SIGNAL(timeout()), this,  SLOT(runDiag()));
	}else{
		   connect(shedDiag,SIGNAL(startJob(int)),this,SLOT(runDiag(int)));
	}

}

sdiagCollector::~sdiagCollector() {
	stopAll();
	if(dcon){
		dcon->dbClose();
		delete(dcon);
	}
	if(toDB && cName!=""){
		delete(tableSData);
		delete(tableDist);
		delete(tableName);
	}

}

void sdiagCollector::stopAll(){
	 shedDiag->clear();
	 JList.clear();
	 if(work) qWarning()<<"stop DIAGN collector";
	 work=false;
	 emit stopAllThread();
}


bool sdiagCollector::start(QDomElement sElem){

	bool rez=false;
	bool ok;
	if(sElem.isNull())            return false;
	if(sElem.nodeName()!="sdiag") return false;
	sdiag=sElem;
	shedDiag->clear();
	JList.clear();

	QString minutes,hours;
	minutes=sdiag.attribute("minutes");
	hours=sdiag.attribute("hours");

	QDomElement sPar;
	QDomNodeList nl=sdiag.childNodes();//число настроек снятия спектра
	int numRms=0;
	for(int i=0;i<nl.size();i++){
		sPar=nl.at(i).toElement();
		QDomNodeList rl=sPar.childNodes();//число расчетов СКЗ
		if(rl.size()==0) continue;

		Job Jn;
		//получаем список устройств
		QString devises;
		devises=sPar.attribute("devises","0");
		if(devises!="0"){//проверка списка
			QStringList s=devises.split(",",QString::SkipEmptyParts);
			for(int n=0;n<s.size();n++){
				unsigned char m=s.at(n).toUShort(&ok);
				if(map->contains(m)) Jn.devList<<m;
			}
		}else{//значит все
			QList<unsigned char> cList=map->keys();
			for(int n=0;n<cList.size();n++) Jn.devList<<cList.at(n);
		}
		Jn.frequency=sPar.attribute("frequency","125");
		Jn.hamming=sPar.attribute("hemming","0").toUShort(&ok);
		Jn.passes=sPar.attribute("passes","0").toUShort(&ok);
		if(sPar.attribute("stype")=="A") Jn.tkan=A;
		if(sPar.attribute("stype")=="C") Jn.tkan=C;

		if(Jn.passes>0){
			for(int j=0;j<rl.size();j++){
				QDomElement rmsPar=rl.at(j).toElement();
				rms Rms;
				Rms.frStep=2.0*Jn.frequency.toUInt(&ok)/maxSizeS;
				Rms.start=rmsPar.attribute("pin","1").toUShort(&ok);
				Rms.size=rmsPar.attribute("size","1").toUShort(&ok);
				if(Rms.start+Rms.size>maxSizeS) Rms.size=maxSizeS-Rms.start;
				Rms.stFr=QString::number(Rms.start*Rms.frStep,'f',3);
				Rms.enFr=QString::number((Rms.start+Rms.size-1)*Rms.frStep,'f',3);
				Rms.vecBins.resize(Rms.size);
				Rms.Name=rmsPar.attribute("rms","Se");
				if(Jn.tkan==C){Rms.nConv=0;	Rms.Razm=sizeRMS.last();}
				if(Jn.tkan==A){
					Rms.nConv=nameRMS.indexOf(Rms.Name);
					Rms.Razm=sizeRMS.at(Rms.nConv);
				}
				Rms.label=rmsPar.attribute("label","");
				Rms.cnum=numRms;
				Jn.rezList.append(Rms);
				numRms++;
			}
			JList.append(Jn);
		}else{
			numRms+=rl.size(); //просто увеличили номер
		}
	}

	//создаем таблицы
	if(toDB){
		dcon=new dbConnector(cName);
		if(dcon->dbConnect(parConn,dbType)) iniTable();
	}

    //запускаем шедулер или таймер цикла
	if(JList.size()>0){
		if(unsheduled){
			 timer.start(1000);
			 rez=true;
			 work=true;
		}else{
			if(shedDiag->append(minutes,hours)){
				rez=true;
				work=true;
				shedDiag->start();
			}
		}
	}

	return rez;
}


tape* sdiagCollector::exe_SDiag(){

	tape* tp=new tape();
	unsigned char fDev; //первое устройство для снятия данных
	dCONNECT *con;
	dIF*  di;
	dFOR* df;
	dDISCONNECT* dd;


    for(int i=0;i<JList.size();i++){//по осцилограммам

    	Job& job=JList[i];
    	if(job.passes==0) continue;//отключено
    	uchar tkan=job.tkan;
    	QList<unsigned char>& dev=JList[i].devList;
    	for(int nk=0;nk<bazeList.size();nk++){//по каналам в устройствах
    		fDev=0;
			for(int j=0;j<dev.size();j++){//по устройствам
				unsigned char m=dev.at(j);
				MArray* mArr=map->value(m);
				if(!mArr->dstatus) continue; //отвалилися блок
				//закладка спектра
				if((uchar)(mArr->getMData(bazeList.at(nk))>>8)!=tkan) continue; //тип не тот

				if(fDev==0) fDev=m;

				//установили соединение, если опрос идет
				con=(dCONNECT*)tp->append(CONNECT);
				con->cType=cEXT;
				con->extExe=mArr->exe;

				di =(dIF*)tp->append(IF);
				di->setAdr(m);
				di->setFun(0x42);
				di->append(0x01);  //закладка
				di->append((char)nk);    //номер канала
				di->append(frList.indexOf(job.frequency));
				di->append(0);
				di->append(0);
				di->append((char)job.passes);
				di->append((char)job.hamming);
				di->addTest(di->getAF());

			}
			//после ожидания
			if(fDev==0) continue; //нечего качать
			fDev=0;
			//попытка скачать
			for(int j=0;j<dev.size();j++){//по устройствам
				unsigned char m=dev.at(j);
				MArray* mArr=map->value(m);
				if(!mArr->dstatus) continue; //отвалилися блок
				if((uchar)(mArr->getMData(bazeList.at(nk))>>8)!=tkan) continue; //тип не тот

				QString Label=QString::number(i)+"."+QString::number(m)+"."+QString::number(nk);
				//установили соединение
				con=(dCONNECT*)tp->append(CONNECT);
				con->cType=cEXT;
				con->extExe=mArr->exe;

				df =(dFOR*)tp->append(FOR);
				df->setAdr(m);
				df->setFun(0x42);
				df->append(0x02);           //статус первого устройства
				if(fDev==0){
					fDev=dev.at(j);
					df->delay=job.passes*5000;  //задержка перед выполнением
					df->nsz=12*3;               //сколько раз ждем (3 мин)
				}else{
					df->after_delay_CON=5000;
					df->nsz=12*3;              // (3 мин)
				}
				df->addTest(di->getAF());
				df->addTest(execut::convert("00"),3);        //готово
				df->continueRez=false;       //повторять при ошибке
				df->stBAD="DISCON_"+Label;

				//выкачка
				QList<rms>& rl=JList[i].rezList;
				for(int d=0;d<rl.size();d++){
					rms& Rms=rl[d];
					//качаем кусками, если надо
					int cou=1+Rms.size/maxSizeM;
					for(int c=0;c<cou;c++){
						di =(dIF*)tp->append(IF,Label+"."+QString::number(d)+"."+QString::number(c));
						di->setAdr(m);
						di->setFun(0x04);
						di->append(execut::convert(startAdr+Rms.start+maxSizeM*c,2));  //базовый
						di->append(0x00);
						if(c+1==cou){
							di->append((char)(Rms.size-maxSizeM*c));  //остаток
						}else{
							di->append((char)maxSizeM);               //максимум 125
						}
						di->addTest(di->getAF());
						di->ControlRez=true;                          // излучать результат

					}
				}
				//пустышка
				dd=(dDISCONNECT*)tp->append(DISCONNECT,"DISCON_"+Label);
			}

    	}
    }

  //  tp->test(true);
	return tp;
}


void sdiagCollector::deleteThred(){

	if(SExeTread* thread = qobject_cast<SExeTread*>(sender())){
		thread->wait();
	    delete(thread);
	    qWarning()<<"sdiag run: end";
		run=false;
	}
 }

bool sdiagCollector::getSDiag(int& err){
	bool rez=false;
	bool ok;
	SExeTread*  eTred=NULL;
    tape* tap;
    tap= exe_SDiag();


    if(tap->size()>0){

    	eTred=new SExeTread(tap);
		connect(this, SIGNAL(stopAllThread()),  eTred, SLOT(halt()));
		connect(eTred, SIGNAL(finished()), this, SLOT(deleteThred()));
		connect(eTred, SIGNAL(emitDate(QString,QByteArray)),this,SLOT(setDate(QString,QByteArray)));


		eTred->moveToThread(eTred);
		eTred->start();

		err=0;
		rez=true;
    }else{
    	delete(tap);
    	err=1;
    }
    return rez;
}

void sdiagCollector::runDiag(int n){

   int err=0;
   int rez=0;
   if(!run){
	   run=rez=getSDiag(err);
	   qWarning()<<"sdiag run:"<<rez<<" err:"<<err;
   }else{
	   if(!unsheduled)  qWarning()<<"sdiag already working!";
   }

}


void sdiagCollector::setDate(QString lab,QByteArray dat){
	bool ok;

	int           i= lab.section('.',0,0).toUInt(&ok);
	unsigned char m= lab.section('.',1,1).toUInt(&ok);
	int           nk=lab.section('.',2,2).toUInt(&ok);
	int           d= lab.section('.',3,3).toUInt(&ok);
	int           c= lab.section('.',4,4).toUInt(&ok);
	Job& job=JList[i];
	rms& Rms=job.rezList[d];
	int cou=1+Rms.size/maxSizeM;
	usort su;
	bool resz=false; //признак переполнения
	for(int v=0;v<dat.size()/2;v++){
		su.sc[1]=dat[2*v];
		su.sc[0]=dat[2*v+1];
		Rms.vecBins[c*maxSizeM+v]=su.usd;
		if(su.usd==0xFFFF) resz=true; //переполнение
	}

	if(c==cou-1){//пришло все
	   //конвертируем величину,если надо
	   double fr=Rms.frStep*Rms.start;
	   double k;
	   double sum=0;
	   if(Rms.nConv==1){
		   for(int v=0;v<Rms.vecBins.size();v++){
			   k=1000.0/(TwoPi*fr);
			   Rms.vecBins[v]*=k;
			   fr+=Rms.frStep;
		   }
	   }
	   if(Rms.nConv==2){
		   for(int v=0;v<Rms.vecBins.size();v++){
			   k=1000.0/(TwoPi*fr);
			   Rms.vecBins[v]*=(k*k);
			   fr+=Rms.frStep;
		   }
	   }

	   //считаем СКЗ
	   for(int v=0;v<Rms.vecBins.size();v++){
		   sum+= Rms.vecBins[v]*Rms.vecBins[v];
	   }


	   short vd;
	   if(resz) vd=-1;//переполнение
	   else vd=qSqrt(sum/2.0)/10.0;

	   int adr;
	   adr=bazeSpList[nk]+Rms.cnum;
	   map->value(m)->setMData(adr,vd);

	   //в базу данных кладем
	   //******************************************************
		if(QSqlDatabase::contains(cName)){
			QSqlDatabase db(QSqlDatabase::database(cName));
			if(db.open()){

				tableSData->setValue(0,QString::number(m));
				tableSData->setValue(1,QString::number(adr));
				tableSData->setValue(2,QString::number(vd));
				tableSData->setValue(3,"now()");
			//	qDebug()<<tableSData->replace();
				QSqlQuery qr(tableSData->replace(),db);
			}
		}
		//*********************************************************

	}


}


void sdiagCollector::iniTable(){

	if(QSqlDatabase::contains(cName)){
		QSqlDatabase db(QSqlDatabase::database(cName));
		if(db.open()){
			QString str;
			//xbcnbv
			QSqlQuery qd1(tableSData->drop(),db);
			QSqlQuery qd2(tableDist->drop(),db);
			QSqlQuery qd3(tableName->drop(),db);

			// Создаем таблицу оперативных данных
			QSqlQuery q1(db);
			if (q1.exec(tableSData->greate())) qDebug()<<"Create database table '"+tableSData->Name+"'";
			// Создаем словарь описания каналов
			QSqlQuery q2(db);
			if (q2.exec(tableDist->greate())) qDebug()<<"Create database table '"+tableDist->Name+"'";
			// Создаем словарь описатия данных
			QSqlQuery q3(db);
			if (q3.exec(tableName->greate())) qDebug()<<"Create database table '"+tableName->Name+"'";


			//заполняем по умолчанию!!!
			str=   "INSERT INTO "+tableName->Name+" VALUES ";
			for(int i=0;i<JList.size();i++){
				Job& job=JList[i];
				for(int j=0;j<job.rezList.size();j++){
					rms& rm=job.rezList[j];
					QString divider="1";
					if(rm.Name==nameRMS[0]) divider="100";
					if(rm.Name==nameRMS[1]) divider="10";

					tableName->setValue(0,QString::number(rm.cnum+1));
					tableName->setValue(1,rm.stFr);
					tableName->setValue(2,rm.enFr);
					tableName->setValue(3,QString::number(rm.frStep,'f',3));
					tableName->setValue(4,divider);
					tableName->setValue(5,"'"+rm.Name+"'");
					tableName->setValue(6,"'"+rm.Razm+"'");
					tableName->setValue(7,"'"+rm.label+"'");
					str+=tableName->getVList()+",";

					/*str+="("+QString::number(rm.cnum+1)+","+
					rm.stFr+","+rm.enFr+","+QString::number(rm.frStep,'f',3)+
					","+divider+",'"+rm.Name+"','"+rm.Razm+"','"+rm.label+"'),";*/

				}
			}
			str[str.size()-1]=';';
			//qDebug()<<str;
			QSqlQuery q4(str,db);
          //*****************************************************************************
          //слоыарь инициализируется после опроса блока (надо знать типы каналов)


          //*****************************************************************************
		}
	}
}

void sdiagCollector::iniDist(int m) {

	if(QSqlDatabase::contains(cName)){
		QSqlDatabase db(QSqlDatabase::database(cName));
		if(db.open()){
			for(int i=0; i < JList.size(); i++){ //по осцилограммам
				Job& job=JList[i];
				if(job.passes == 0) continue; //отключено ???
				uchar tkan=job.tkan;
				QList<unsigned char>& dev=JList[i].devList;
				for(int nk=0; nk < bazeList.size(); nk++){ //по каналам в устройствах
					for(int j=0; j < dev.size(); j++){ //по устройствам
						if(m == dev.at(j)){ // для заданного канала
							MArray* mArr=map->value(m);
							if(!mArr->dstatus) continue; //отвалилися блок
							//закладка спектра
							if((char) (mArr->getMData(bazeList.at(nk)) >> 8) != tkan) continue; //тип не тот

							QList<rms>& rl=job.rezList;
							for(int d=0; d < rl.size(); d++){
								int nd=rl[d].cnum;
								int adr=bazeSpList[nk] + nd;
								QString T="A";
								if(tkan == C) T="C";
								tableDist->setValue(0, QString::number(m));
								tableDist->setValue(1, QString::number(adr));
								tableDist->setValue(2, QString::number(nk + 1));
								tableDist->setValue(3, "'" + T + "'");
								tableDist->setValue(4, QString::number(nd + 1));
								//qDebug()<<zp;
								QSqlQuery q(tableDist->replace(), db);
							}
						}
					}
				}
			}
		}
	}
}



void sdiagCollector::setDiap(QDomNode node){
	bool ok;
	if(node.nodeName()=="diap"){
		QDomElement parent=node.parentNode().toElement();
		QDomElement el=node.toElement();
		float step=2.0*parent.attribute("frequency").toUInt(&ok)/maxSizeS;
		float st=step*el.attribute("pin").toUInt(&ok);
		int  sz=el.attribute("size").toUInt(&ok);
		float en=st+step*(sz-1);
		QString stp=QString::number(st,'f',3);
		if(sz>1) stp=stp+"-"+QString::number(en,'f',3);
		el.setAttribute("band",stp+" Hz");
	}
}

void sdiagCollector::setAC(QDomNode node){
	bool ok;
	if(node.nodeName()=="A" || node.nodeName()=="C"){
		QDomElement el=node.toElement();
		float step=2.0*el.attribute("frequency").toUInt(&ok)/maxSizeS;
		el.setAttribute("resolution",QString::number(step,'f',3));
		QDomNodeList nl=el.childNodes();
		for(int i=0;i<nl.size();i++) setDiap(nl.at(i));
	}
}

//******************************************************************************

void SExeTread::exeRez(command* cmd){

	if(cmd->Type==IF ){
		dIF* di=(dIF*)cmd;
		if(di->fun()==0x04 && di->getRez()==OK){
			int sz=di->data[2];
			emit emitDate(di->Label,di->data.mid(3,sz));
		}
	}
}






