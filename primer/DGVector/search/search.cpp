#include "search.h"
#include <QtCore>

//emiterS*  search::Emit = 0;

search::search(deditor* edit,QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);

	editor=qobject_cast<deditor*>(edit);  //получаем указатель на родителя как на обьект типа deditor
	if(!editor) return;
    exe.enDebug(SYSTEM::NumDebug);

    #ifdef _WIN_
		listCOM << "COM1" << "COM2" << "COM3" << "COM4" << "COM5" << "COM6" << "COM7" << "COM8"<< "COM9";
	#else
		listCOM << "ttyS0" << "ttyS1" << "ttyS2" << "ttyS3"<< "ttyS4";
		listCOM << "ttyUSB0" << "ttyUSB1" << "ttyUSB2" << "ttyUSB3"<< "ttyUSB4";
	#endif /*_TTY_POSIX*/
	ui.comboBox_ComPort->addItems(listCOM);
	BaudRate<<"9600"<<"19200"<<"38400"<<"57600"<<"115200";
	ui.comboBox_BaudRate->addItems(BaudRate);
	isCOM=true;


	setBroadcast(ui.checkBox_Broadcast->isChecked());

	setInterfaseToCOM(ui.radioButton_COM->isChecked());
	connect(ui.radioButton_COM, SIGNAL(toggled(bool)),this,SLOT(setInterfaseToCOM(bool)));
	connect(ui.pushButton_SEARCH,SIGNAL(toggled(bool)),this,SLOT(start(bool)));
	connect(ui.checkBox_Broadcast,SIGNAL(toggled(bool)),this,SLOT(setBroadcast(bool)));
	connect(&exe,SIGNAL(statScharman(bool)),this,SLOT(stop(bool)));

/*
	connect(getEmit(),SIGNAL(addSearchNodeS(quint32,const QByteArray&)),this,SLOT(addNode(quint32,const QByteArray&)));
	connect(getEmit(),SIGNAL(addVersionNodeS(const QString&,const QByteArray&,const QByteArray&)),this,SLOT(addVersion(const QString&,const QByteArray&,const QByteArray&)));
	connect(&exe,SIGNAL(messageOK(QString)),this,SLOT(setOK(QString)));
	connect(&exe,SIGNAL(messageBAD(QString)),this,SLOT(setBAD(QString)));
*/
	connect(&exe,SIGNAL(rezult(command*)),this,SLOT(headerRez(command*)));


	QString ipmask="(25[0-5]|2[0-4][0-9]|[0-1][0-9]{2}|[0-9]{2}|[0-9])\\.(25[0-5]|2[0-4][0-9]|[0-1][0-9]{2}|[0-9]{2}|[0-9])\\.(25[0-5]|2[0-4][0-9]|[0-1][0-9]{2}|[0-9]{2}|[0-9])\\.(25[0-5]|2[0-4][0-9]|[0-1][0-9]{2}|[0-9]{2}|[0-9])";
	QRegExp rx(ipmask);
	QValidator *validator = new QRegExpValidator(rx, this);
	ui.lineEdit_IP1->setValidator(validator);
	ui.lineEdit_IP2->setValidator(validator);

}


search::~search()
{
	exe.Stop();
}

/*
 * Выбор интерфейса
 */
void search::setInterfaseToCOM(bool isCom){

	ui.label_IP->setVisible(!isCom);
	ui.label_Broadcast->setVisible(!isCom);
	ui.checkBox_Broadcast->setVisible(!isCom);
	ui.lineEdit_IP1->setVisible(!isCom);
	ui.lineEdit_IP2->setVisible(!isCom);
	ui.comboBox_ComPort->setVisible(isCom);
	ui.comboBox_BaudRate->setVisible(isCom);
	isCOM=isCom;
}

int search::testIP(QString ip){
	int rez=0;
	if(ip!=""){
		QDomNodeList nodeList=editor->rootNode.elementsByTagName ("node");
		for(int i=0;i<nodeList.count();i++){
			if(ip==nodeList.at(i).toElement().attribute("ip")) rez++;
		}
	}
	return rez;
}

int search::testID(QString id){
	int rez=0;
	if(id!=""){
		if(editor->MapIdStatus.contains(id)){
			return editor->MapIdStatus[id];
		}
	}
	return rez;
}


/*
 * Получение информации о блоках
 */
void search::getInfo(){
	emit Run();
	exe.setTape(); //готовим шарманку
	// получить список блоков

	qint16 udp_port=(int)rand() % 16383 + 49152;//откуда шлем для UDP
	QString     dt;
	QDomNode curnode;
	QString comPort,bitrate,ip,modbus,id,dir,master;
	QDomNodeList nodeList=editor->rootNode.elementsByTagName ("node");
	if(nodeList.count()==0){
		Stage=ParseInfo_2;
		stop(false);
		return;
	}
	Stage=GetInfo;
	bool ok;
	unsigned int wait=100;

	dIF* di;
//	dSETTIME* dst;
	//dFOR* df;
//	dDOWNLOAD* dd;
	dCONNECT*  dco;

	//-------------------------------------------


	for(int i=0;i<nodeList.count();i++){

		curnode=nodeList.at(i);
		comPort=curnode.toElement().attribute("port");
		bitrate=curnode.toElement().attribute("bitrate");
		ip=curnode.toElement().attribute("ip");
		modbus=curnode.toElement().attribute("modbus");
		id=curnode.toElement().attribute("id","");
		master=curnode.toElement().attribute("master");
		dir=XSD::appPath+"/CFG/"+id;
		QDir().mkpath(dir);
		unsigned char m=exe.convert(modbus.toInt(&ok,10),1).at(0);


		dco=(dCONNECT*)exe.append(CONNECT); 					//добавили команду
		if(comPort==""){
			if(testIP(ip)>1 && master=="0" ){//для слейвов за мастером
				dco->cType=cTCP;
				wait=300;
			}else{
				dco->cType=cUDP;
				dco->udpSAddr="0.0.0.0";  //адрес
				dco->udpSPort=udp_port;  //порт
				wait=100;
			}
			dco->cAddr=ip;      //адрес
			dco->cParam="502";  //порт

		}else{
			dco->cType=cCOM;
			dco->cAddr=comPort;
			dco->cParam=bitrate;
			wait=300;
		}
		dco->setBAD(-1);

		//----------------------------
/*
		//установка времени текущего
		dst=(dSETTIME*)exe.append(SETTIME);
		dst->wait=1000;//
		dst->setAdr(m); //адреc
		dst->stOK=id;
		dst->stBAD=id;
		dst->ControlRez=true;
*/
		//------------------------------------------------------

		di =(dIF*)exe.append(IF,id);
		di->setAFCmd(m,0x2B,execut::convert("0E0401"));
		di->addTest(di->getAF(),0);
		di->ControlRez=true;
		di->stOK=id.toUpper();
		di->stBAD=id;

		di =(dIF*)exe.append(IF);
		di->setAFCmd(m,0x2B,execut::convert("0E0402"));
		di->addTest(di->getAF(),0);
		di->ControlRez=true;
		di->stOK=id.toUpper();

		//di->fDAT=updateDataS;
		//----------------------------

		//выкачка файла
/*
		dd=(dDOWNLOAD*)exe.append(DOWNLOAD);
		//dd->wait=1000;//
		dd->setAFCmd(m,0x68,exe.convert("0000"));
		dd->dirName=dir;
		dd->shortName="CFG.TXT";
		dd->msize=6;
		dd->wait=wait;
*/
		//----------------------------
		//закрытие порта

		exe.append(DISCONNECT);

		//-------------------------------------------------------
	}

	if(exe.getTape()->count()>0)	exe.Start();
	else                            stop(false);
}


/*
 * читаем данные из файлов конфигурации - по событиям и лутам
 */
void search::parseInfo_1(){


	QDomElement curnode;
	QString port,bitrate,ip,modbus,id,dir,master;
	QString Name,txt;
	QDomNodeList nodeList=editor->rootNode.elementsByTagName ("node");
	qint16 udp_port=(int)rand() % 16383 + 49152;//откуда шлем для UDP
	bool ok;


		dDOWNLOAD* dd=NULL;
		dCONNECT*  dco;


	if(nodeList.count()==0){
		qDebug()<<"Not Node for info";
			//stop(false);
			return;
	}
	exe.setTape(); //готовим шарманку
	QMap<QString,dovn>       dovnMap; //что качаем дополнительно
	QStringList              ListID;
	for(int i=0;i<nodeList.count();i++){
			curnode=nodeList.at(i).toElement();
			id=curnode.attribute("id");
			//qDebug()<<id;
			QDir().mkpath(XSD::appPath+"/CFG/"+id);
			Name=XSD::appPath+"/CFG/"+id+"/CFG.TXT";
        	if(arrs.ReadCFG(Name)){
        		Name="GEFILE";
       			dovnMap[id].ge=arrs[Name].p[0];
       			if(dovnMap[id].ge.size()>0 && dovnMap[id].ge!="NONE" && dovnMap[id].ge!="0"){
       				if(!ListID.contains(id)) ListID<<id;
       			}

        		for(int j=0;j<5;j++){
        			Name="LUT"+txt.setNum(j+1,10);
        			dovnMap[id].lut<<arrs[Name].p[0];

        			if(dovnMap[id].lut.at(j).size()>0 && dovnMap[id].lut.at(j)!="NONE" && dovnMap[id].lut.at(j)!="0"){
        				if(!ListID.contains(id)) ListID<<id;
        			}


        		}
        	}
	}


	for(int i=0;i<nodeList.count();i++){
		    curnode=nodeList.at(i).toElement();
		    id=curnode.attribute("id");
   			if(testID(id)<0 || !ListID.contains(id))  continue;

			port=curnode.attribute("port");
			bitrate=curnode.attribute("bitrate");
			ip=curnode.attribute("ip","");
			modbus=curnode.attribute("modbus");
			master=curnode.toElement().attribute("master");

			dir=XSD::appPath+"/CFG/"+id;


			dco=(dCONNECT*)exe.append(CONNECT); 					//добавили команду
			if(port==""){
				if(testIP(ip)>1 && master=="0" ){//для слейвов за мастером
					dco->cType=cTCP;
				}else{
					dco->cType=cUDP;
					dco->udpSAddr="0.0.0.0";  //адрес
					dco->udpSPort=udp_port;  //порт
				}
				dco->cAddr=ip;      //адрес
				dco->cParam="502";  //порт
			}else{
				dco->cType=cCOM;
				dco->cAddr=port;
				dco->cParam=bitrate;
			}

			dco->setBAD(-1);
			//----------------------------

			if(dovnMap[id].ge.size()>0 && dovnMap[id].ge!="NONE" && dovnMap[id].ge!="0"){



				//выкачка файла
				dd=(dDOWNLOAD*)exe.append(DOWNLOAD);
				dd->wait=1000;//
				dd->setAFCmd(exe.convert(modbus.toInt(&ok,10),1).at(0),105,exe.convert("0000"));
				dd->dirName=dir;
				dd->msize=6;
				dd->shortName=dovnMap[id].ge;
				//----------------------------
			}
			//--------------------------------
			for(int j=0;j<dovnMap[id].lut.count();j++){
				if(dovnMap[id].lut.at(j).size()>0 && dovnMap[id].lut.at(j)!="NONE" && dovnMap[id].lut.at(j)!="0"){
					//выкачка файла
					dd=(dDOWNLOAD*)exe.append(DOWNLOAD);
					dd->wait=1000;//
					dd->setAFCmd(exe.convert(modbus.toInt(&ok,10),1).at(0),106,QByteArray(1,j)+=exe.convert("0000")); //адрес
					dd->dirName=dir;
					dd->msize=6;
					dd->shortName=dovnMap[id].lut.at(j);
					//qDebug()<<dd->shortName;
					//----------------------------

				}
			}
		}
	    //-------------------------------------
		//закрытие порта

	if(exe.getTape()->count()>0){
		exe.append(DISCONNECT);
		exe.Start();
	}else stop(false);

}

/*
 * читаем данные из файлов конфигурации и пишем в редактор
 */
void search::parseInfo_2(){

	QDomElement curnode,nev;
	QString id;
	QString Name,txt;
	QDomNodeList nodeList=editor->rootNode.elementsByTagName ("node");
	if(nodeList.count()==0){
		qDebug()<<"Not Node for info";
			//stop(false);
			return;
	}


	//идем на родителя в дереве - чтоб обновить атрибуты
	editor->goParent(editor->indexTreeFromNode(nodeList.at(0).toElement()));

	for(int i=0;i<nodeList.count();i++){
			curnode=nodeList.at(i).toElement();
			QModelIndex ti=editor->indexTreeFromNode(curnode);
			//очистка узлов
			//editor->clearNode(ti);

			QStringList addList=editor->addNodeList(ti);

			id=curnode.attribute("id");
			Name=XSD::appPath+"/CFG/"+id+"/CFG.TXT";

			QStringList cfgNodeName;//какие узлы описаны в конфиге
        	if(arrs.ReadCFG(Name)){
        		//имя конфигурации
        		curnode.setAttribute("config",arrs["ID"].p[0]);
        		//тахометры
        		if(arrs.contains("TAH.NAME")){
					for(int i=0;i<arrs["TAH.NAME"].p.count();i++){
						Name="TAH"+txt.setNum(i+1,10);
						if(arrs[Name+".CTZ"].p[1]!="0"){
							cfgNodeName<<Name;
							if(addList.contains(Name))	nev=editor->addNode(ti,Name);
							else nev=curnode.namedItem(Name).toElement();
							if(nev.isNull()) continue;
							if(arrs["TAH.NAME"].p[i]!="NONAME"){
								nev.setAttribute("name",arrs["TAH.NAME"].p[i]);
							}else{
								nev.setAttribute("name","");
							}

						}
					}
        		}
        		//каналы
        		if(arrs.contains("ADC.OUT")){
					for(int i=0;i<arrs["ADC.OUT"].p.count();i++){
						if(arrs["ADC.OUT"].p[i]!="0"){
							Name="ADC"+txt.setNum(i+1,10);
							cfgNodeName<<Name;
							if(addList.contains(Name)) 	nev=editor->addNode(ti,Name);
							else  nev=curnode.namedItem(Name).toElement();
							if(nev.isNull()) continue;
							nev.setAttribute("type",arrs["ADC.OUT"].p[i]);
							if(arrs["ADC.NAME"].p[i]!="NONAME"){
								nev.setAttribute("name",arrs["ADC.NAME"].p[i]);
							}else{
								nev.setAttribute("name","");
							}
						}
					}
        		}
        		//виртуальные каналы
        		if(arrs.contains("VIR.OUT")){
					for(int i=0;i<arrs["VIR.OUT"].p.count();i++){
						if(arrs["VIR.OUT"].p[i]!="0"){
							Name="VIR"+txt.setNum(i+1,10);
							cfgNodeName<<Name;
							if(addList.contains(Name))	nev=editor->addNode(ti,Name);
							else nev=curnode.namedItem(Name).toElement();
							if(nev.isNull()) continue;
							nev.setAttribute("type",arrs["VIR.OUT"].p[i]);
							if(arrs["VIR.NAME"].p[i]!="NONAME"){
								nev.setAttribute("name",arrs["VIR.NAME"].p[i]);
							}else{
								nev.setAttribute("name","");
							}
						}
					}
        		}

        		//тахометры
				if(arrs.contains("TERMO.NAME")){
					for(int i=0;i<arrs["TERMO.NAME"].p.count();i++){
						Name="TERMO"+txt.setNum(i+1,10);
						if(arrs[Name+".INI"].p[1]!="0"){
							cfgNodeName<<Name;
							if(addList.contains(Name))	nev=editor->addNode(ti,Name);
							else nev=curnode.namedItem(Name).toElement();
							if(nev.isNull()) continue;
							if(arrs["TERMO.NAME"].p[i]!="NONAME"){
								nev.setAttribute("name",arrs["TERMO.NAME"].p[i]);
							}else{
								nev.setAttribute("name","");
							}

						}
					}
				}

        		//теперь удаляем отсутствующее в списке
        		QDomNodeList cildList=curnode.childNodes();
        		int i=0;
        		while(i<cildList.size()){
        			if(!cfgNodeName.contains(cildList.at(i).nodeName())){
        				editor->deleteNode(editor->indexTreeFromNode(cildList.at(i).toElement()));
        				cildList=curnode.childNodes();
        				i=0;
        			}else i++;
        		}
        	}
        	editor->goElement(curnode);
	}
	editor->goParent(editor->indexTreeFromNode(nodeList.at(0).toElement()));
}

/*
 * Поиск
 */
void search::start(bool st){

	 interfaceEnable(!st);
     if(st){
    	 emit Run();
    	 QFile file;
    	 QString NDoc;
    	 NDoc=editor->getNameXml();
    	// if(file.copy(NDoc,NDoc+"~")) OldFile=NDoc+"~";

    	  if(!exe.getStatus()){//не запущена - запускаем
    		 Stage=Search;
    		 //очистка окна поиска - не чистим - добавляем!!!
    		 // if(editor)	editor->clearNode(editor->indexTreeFromNode(editor->rootNode));

			 if(ui.radioButton_COM->isChecked()){
				 isCOM=true;
				 searchCOM();
			 }else if(ui.radioButton_LAN->isChecked()){
				 isCOM=false;
				 Masters.clear();
				 searchLAN();
			 }
    	 }
     }else{//останов
    	 if(exe.getStatus()) exe.Stop();
     }
}

/*
 * Второй этап поиска по slave
 */
void search::start2(){
	   exe.setTape();

		dIF* di;
		dCONNECT*  dco;


		QHostAddress   masterAdr;

		QString txt,ip;
		QList<QString> IpList;
		unsigned char mb;


		if(Masters.count()==0) return;
		else{

			IpList=Masters.keys();
		}

	    //--------------------------------------------------------
		//закладываем по всем адресам
		for(int k=0;k<IpList.count();k++){
			ip=IpList.at(k);
			mb=Masters[ip];

			dco=(dCONNECT*)exe.append(CONNECT); 					//добавили команду
			dco->cType=cTCP;
			dco->cAddr=ip;      //адрес
			dco->cParam="502";  //порт
			dco->goBAD_L="DISKONNECT"+txt.setNum(k);

			for(int i=ui.spinBox_1->value();i<=ui.spinBox_2->value();i++){//
				if(i==mb) continue;

				di=(dIF*)exe.append(IF); //добавили команду
				di->delay=1;//задержка, чтоб запросы успевали уходить
				di->wait=100;//ждем ответ
				di->setAFCmd(exe.convert(i,1).at(0),103,exe.convert("00"));
				di->msize=11; //ответ не менее
				di->addTest(di->getAF());
				di->ControlRez=true;
				//di->fOK=funcOK;
			}
			//--------------------------------------------------
			exe.append(DISCONNECT,"DISKONNECT"+txt.setNum(k));
			//-------------------------------------------------------
		}
		Masters.clear();

		if(exe.getTape()->count()>0)	exe.Start();
		else                            stop(false);
}

void search::stop(){
	exe.Stop();
}

/*
 * Окончание работы шарманки
 */
void search::stop(bool st){
	 if(!st){
	   	 if(exe.getStatus()) exe.Stop();
	   	 switch(Stage){
			 case Search:
				 if(!isCOM && Masters.count()>0){
					 Stage=SearchSlave;
				 	 start2();
				 }else{
					 Stage=GetInfo;
					 getInfo();
				 }
				 break;
			 case SearchSlave:
				 Stage=GetInfo;
				 getInfo();
				 break;
			 case GetInfo:
				// Stage=ParseInfo_1;
				// parseInfo_1();
				 ui.pushButton_SEARCH->setChecked(false);
				 Stage=ParseInfo_2;
				 emit Done();
				 break;
			 case ParseInfo_1:
				 Stage=ParseInfo_2;
				 parseInfo_2();
				 ui.pushButton_SEARCH->setChecked(false);
				 emit Done();
				 break;
			 case ParseInfo_2:
				 ui.pushButton_SEARCH->setChecked(false);
				 emit Done();
	   	 }
	 }
}


void search::headerRez(command* cmd){

/*	if(cmd->getType()==SETTIME){
		if(cmd->getRez()==OK) setOK(cmd->stOK);
		else                  setBAD(cmd->stBAD);
		return;
	}
*/
	if(cmd->getType()==IF){
		 //if(cmd->getRez()!=OK) return;
		 if(cmd->getRez()==OK){ setOK(cmd->stOK.toLower());}
		 else                 { setBAD(cmd->stBAD); return;}
		 dIF* di=(dIF*)cmd;
		 switch(Stage){
			 case Search:
				 addNode(di->AddrRF.toIPv4Address(),di->getData());
				 break;
			 case SearchSlave:
				 addNode(di->AddrRF.toIPv4Address(),di->getData());
				 break;
			 case GetInfo:
				 addVersion(di->stOK,di->getAFCmd(),di->getData());
				 break;
			 case ParseInfo_1:
			 case ParseInfo_2:
				  break;
		 }
		 return;
	 }

	 if(cmd->getType()==MULTICAST && Stage==Search){
		 dMULTICAST* dm=(dMULTICAST*)cmd;
		 QList<quint32> key=dm->multiArray.keys();
		 for(int i=0; i<key.size();i++){
			 addNode(key[i],dm->multiArray[key[i]]);
		 }
	 }
}

void search::setBroadcast(bool br){
	ui.lineEdit_IP1->setEnabled(!br);
	ui.lineEdit_IP2->setEnabled(!br);
	Broadcast=br;
}

/*
 * Доступность управления
 */
void search::interfaceEnable(bool en){
	ui.radioButton_COM->setEnabled(en);
	ui.radioButton_LAN->setEnabled(en);
	ui.comboBox_ComPort->setEnabled(en);
	if(!Broadcast){
		ui.lineEdit_IP1->setEnabled(en);
		ui.lineEdit_IP2->setEnabled(en);
	}
	ui.spinBox_1->setEnabled(en);
	ui.spinBox_2->setEnabled(en);
	ui.comboBox_BaudRate->setEnabled(en);
	ui.checkBox_Broadcast->setEnabled(en);
	if(en==true){
		ui.pushButton_SEARCH->setEnabled(en);
	}
}

/*
 * Поиск по COM
 */

void search::searchCOM(){



	exe.setTape();

	dIF* di;
	dCONNECT*  dco;

	bool ok;
	int wait;
	//время реакции блока и парсинга + время приема/получения 22 байт запрос ответ
	wait=10+2*115200/ui.comboBox_BaudRate->currentText().toInt(&ok,10);

#ifdef _WIN_
	wait+=20;
#endif


	//-------------------------------------------

	dco=(dCONNECT*)exe.append(CONNECT); 					//добавили команду
	dco->cType=cCOM;
	dco->cAddr=ui.comboBox_ComPort->currentText();
	dco->cParam=ui.comboBox_BaudRate->currentText();
	dco->setBAD(-1);

    //--------------------------------------------------------
	//закладываем по всем адресам
	for(int i=ui.spinBox_1->value();i<=ui.spinBox_2->value();i++){//

		//готовность буфера
		di=(dIF*)exe.append(IF); 	 					//добавили команду
		di->wait=wait;
		di->setAFCmd(exe.convert(i,1).at(0),103,exe.convert("00"));
		di->addTest(di->getAF());
		di->ControlRez=true;
		//di->fOK=funcOK;
	}
    //--------------------------------------------------

	if(exe.getTape()->count()>0){
		exe.append(DISCONNECT);
		exe.Start();
	}else stop(false);

}

void search::searchLAN(){
	//очистка окна поиска

	exe.setTape();


	dCONNECT*  dco;
	dMULTICAST*  dm;

	//bool ok;
	QHostAddress       udpAdr,startAdr,stopAdr,broadcastAdr;
	broadcastAdr.setAddress("255.255.255.255");
	startAdr.setAddress(ui.lineEdit_IP1->text());
	stopAdr.setAddress(ui.lineEdit_IP2->text());
	int port=rand() % 16383 + 49152; //случайно из диапазона 49152 — 65535
	QString txt;
	quint64 i,n,k;


	//-------------------------------------------

	dco=(dCONNECT*)exe.append(CONNECT); 					//добавили команду
	dco->cType=cUDP;
	dco->udpSAddr="0.0.0.0";             //слушаем ответы отовсюду
	dco->udpSPort=port;                  //на каком порту слушаем
	dco->cParam=txt.setNum(udpPort,10);  //Куда шлем
	dco->setBAD(-1);

    //--------------------------------------------------------
	if(!Broadcast){
		n=startAdr.toIPv4Address();
		k=stopAdr.toIPv4Address();
	}else{
		n=k=broadcastAdr.toIPv4Address();
	}

    //--------------------------------------------------

	dm=(dMULTICAST*)exe.append(MULTICAST);
	dm->setAFCmd(exe.convert(0,1).at(0),0x67,exe.convert("00")); //броадкаст
	dm->wait=1000; //ждем прихода ответов
	//закладываем по всем адресам
	for(i=n;i<=k;i++){
		udpAdr.setAddress((qint32)i);
		dm->appendAdr(udpAdr.toString());
	}

	dm->msize=11; //ответ не менее
	dm->addTest(dm->mid(1,1),1);
	dm->ControlRez=true;
	//dm->fOK=funcREZ;


	if(exe.getTape()->count()>0){
		exe.append(DISCONNECT);
		exe.Start();
	}else stop(false);
}

void search::addNode(quint32 ip32,const QByteArray& rez){

	//qDebug()<<"!!!!!!"<<execut::ByteArray_to_String(rez)<<ip32;

	if(!editor) return;
    QString id;
    QString ip=QHostAddress(ip32).toString();
    uint modbus,master;

    id=execut::ByteArray_to_String(rez.mid(2,8));
    if(id.left(8)=="00000000"){
    	QString con;
		QString nc="7c";
		QString nu;
		ushort n;
		if(isCOM){
			con=ui.comboBox_ComPort->currentText().toLower();
			if(con.left(3)=="com")     n=con.mid(3).toUShort();
			if(con.left(4)=="ttys")    n=con.mid(4).toUShort();
			if(con.left(6)=="ttyusb"){ n=con.mid(6).toUShort(); nc="7b";}
			id.replace(0,2,nc);
			nu=QString::number(n,16);
			while(nu.size()<2) nu.prepend("0");
			id.replace(2,2,nu);
		}else{
			id.replace(0,2,"7a");
			nu=QString::number(QHostAddress(ip).toIPv4Address(),16);
			while(nu.size()<8) nu.prepend("0");
			id.replace(2,8,nu);
		}
    }

    modbus=(unsigned char)rez.at(0);
    master=(unsigned char)rez.at(10);

    QDomElement nEl;
	QStringList AN=editor->schema->getChildName(editor->rootNode);

	//а нет-ли  такого элемента?
	if(AN.contains("node")){
		QDomNodeList cild=editor->rootNode.childNodes();
		for(int i=0;i<cild.size();i++){
			if(cild.at(i).toElement().attribute("id")==id){
				nEl=cild.at(i).toElement();
			}
		}
		if(nEl.isNull()) nEl=editor->addNode(editor->indexTreeFromNode(editor->rootNode),"node");
	}
	if(nEl.isNull()) return;

    //список возможных атрибутов
	QStringList AA=editor->schema->getAttributesName(nEl,getAll);
	if(AA.contains("id")) nEl.setAttribute("id",id);
	if(AA.contains("modbus")) nEl.setAttribute("modbus",modbus);
	if(AA.contains("master")) nEl.setAttribute("master",master);
	if(!isCOM){
		switch(Stage){
		case Search:
			if(AA.contains("ip")) nEl.setAttribute("ip",ip);
			//а не мастер-ли?
			if(master==1) Masters[ip]=modbus;
			break;
		case SearchSlave:
			if(AA.contains("ip")) nEl.setAttribute("ip",exe.getAddress());
			break;
		case GetInfo:
		case ParseInfo_1:
		case ParseInfo_2:
			break;
		}
		nEl.removeAttribute("port");
		nEl.removeAttribute("bitrate");
	}else{
		if(AA.contains("port"))    nEl.setAttribute("port",ui.comboBox_ComPort->currentText());
		if(AA.contains("bitrate")) nEl.setAttribute("bitrate",ui.comboBox_BaudRate->currentText());
		nEl.removeAttribute("ip");
	}

}


/*
 * Можно сгенерить сигнал из глобального сигнализатора
 */


//Ловим идентификаторы найденного и не найденного
void search::setOK(QString id){
	if(QFile(XSD::appPath+"/CFG/"+id+"/CFG.TXT").remove()){
		qDebug()<<"delete old: "+XSD::appPath+"/CFG/"+id+"/CFG.TXT";
	}
	editor->setIdStatus(id,1);
}

void search::setBAD(QString id){
	editor->setIdStatus(id,-1);
}


void search::addVersion(const QString& id,const QByteArray& sr,const QByteArray& re){


	if(sr[1]!=0x2B || re[1]!=0x2B) return;

	QString str;
	QDate vdat;
	QLocale loc(QLocale::English, QLocale::UnitedStates);
	bool isD=false;
	bool isN=false;

	if(sr[4]==02){//версия прошивки

		if(re[2]==0x0E){//это корректно
			str=QString::fromLatin1((re.data()+10),(int)re[9]);
		}else{//это затычка для ошибке в прошивке
			str=QString::fromLatin1((re.data()+10-1),(int)re[9-1]);
		}
		str.replace(QString("  "),QString(" "));
		str.replace(QString(" 0"),QString(" "));
		vdat=loc.toDateTime(str,"MMM d yyyy").date();
		isD=true;
	}

	if(sr[4]==01){//имя устройства
		if(re[2]==0x0E){//это корректно
			str=QString::fromLatin1((re.data()+10),(int)re[9]);
		}else{//это затычка для ошибке в прошивке
			str=QString::fromLatin1((re.data()+10-1),(int)re[9-1]);
		}
		isN=true;
	}

	QDomElement n;
	QDomNodeList l=editor->rootNode.elementsByTagName ("node");
	//идем на родителя в дереве - чтоб обновить атрибуты
	editor->goParent(editor->indexTreeFromNode(l.at(0).toElement()));

	for(int i=0; i<l.size();i++){
		n=l.at(i).toElement();
		if(n.attribute("id").toUpper()==id){
			 //список возможных атрибутов
			QStringList AA=editor->schema->getAttributesName(n,getOtional);
			if(isD){
				if(AA.contains("version")){
					n.setAttribute("version",vdat.toString("yyyy-MM-dd"));
				}
			}
			if(isN){
				if(AA.contains("nameDev")){
					n.setAttribute("nameDev",str);
				}
			}
		}
	}

}



