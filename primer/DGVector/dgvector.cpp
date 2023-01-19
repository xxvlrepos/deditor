#include "dgvector.h"
#include <QApplication>
#include <QCloseEvent>
#include <server.h>
#include <sdiagCollector.h>


bool dgvector::Debug=false;
logO* logO::m_instance = 0;

dgvector::dgvector(QWidget *parent) : QMainWindow(parent)
{
	//версия совта
	prName="DGVector";
    ver=server::ver;
	prName+=" V."+ver+"  ";

	isRun=-1;
	isParent=-1;
	runEv=true;
	logDT=QDateTime::fromTime_t(0);
	runStd=0;
	settings = new QSettings("dgvector.ini",QSettings::IniFormat,this);
	ServerHost=settings->value("/settings/ServerHost","localhost").toString();

	Url.setUrl("http://"+ServerHost+":"+QString::number(RPC::Port)+"/RPC2");
	if(ServerHost=="" || !Url.isValid()) ServerHost=QHostAddress::LocalHost;
	QUrl ur("http://"+ServerHost+":"+QString::number(RPC::Port)+"/RPC2");
	Url.swap(ur);

	mdiArea = new QMdiArea;
	mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setCentralWidget(mdiArea);
	//connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)),this, SLOT(updateMenus()));
	 //отчет
	 informer=NULL;
//	 dataCan=NULL;//получить данные
	 searchDevices=NULL;
	 helpBr=NULL;
	 tuner=NULL;
	 VarningUpdate=NULL;
	 getStatThr=setConfThr=NULL;

	 createActions();
	 createMenu();
	 createToolBar();
	 createStatusBar();

	 createDockWindows();

	 setTrayIconActions();
	 showTrayIcon();

	 setWindowTitle(prName);
	 resize(800, 600);

	 //сечем время поиска

	 connect(&timerState, SIGNAL(timeout()), this, SLOT(getStatusRPC()));

	 rpcXmlCl = new MaiaXmlRpcClient(Url, this);
	 connectToServer(Url.host());

	// show();
}

dgvector::~dgvector()
{

	if(informer) delete(informer);
	if(rpcXmlCl) delete(rpcXmlCl);
}

void dgvector::connectToServer(QString addr){

	rpcXmlCl->stop();
	if(isRun!=-1) closeConnectToServer();
	if(addr=="") return;
    QUrl nUrl("http://"+addr+":"+QString::number(RPC::Port)+"/RPC2");
    if(nUrl.isValid()){
    	if(nUrl.host()!=Url.host()){
    		Url.swap(nUrl);
    		rpcXmlCl->setUrl(Url);
    		//qDebug()<<Url.toString();
    	}
    	getStatThr=rpcXmlCl->call("server.getVer",QVariantList(),
    							this,SLOT(versionTest(const QVariant&)),
    							this,SLOT(maiaERR(int, const QString &)),3000);
	/*
    	getStatThr=rpcXmlCl->call("server.isRun",QVariantList(),
						this,SLOT(setStatusSRV(const QVariant&)),
						this,SLOT(maiaERR(int, const QString &)),3000);
	*/

    }
}

void dgvector::versionTest(const QVariant& v){
	 if(v.toString()==ver){
		 getStatThr=rpcXmlCl->call("server.isRun",QVariantList(),
				 	 	 	 	 	 this,SLOT(setStatusSRV(const QVariant&)),
									 this,SLOT(maiaERR(int, const QString &)),3000);
	 }else{
		 if(tuner){
			 tuner->killProcess();
			 tuner->hide();
		 }
		 rpcXmlCl->stop();
		 QMessageBox::warning(
		 				this,
		 				tr("Error"),
		 				tr("Wrong server version (%1)!").arg(v.toString()),
		 				QMessageBox::Close,QMessageBox::Close);
	 }
}

void dgvector::createActions(){

	quitAct = new QAction(QIcon(":/pic/resource/pic/exit.png"),tr("&Quit"),this);
	quitAct->setShortcut(tr("Ctrl+Q"));
	quitAct->setStatusTip(tr("Closing the program")); //всплывающая подсказка
	quitAct->setToolTip(tr("Closing the program"));   //в строке состояния
	connect(quitAct, SIGNAL(triggered()),this,SLOT(close()));

	aboutAct = new QAction(tr("&About"), this);
	aboutAct->setStatusTip(tr("Show the application's About box"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

	aboutQtAct = new QAction(tr("About &Qt"), this);
	aboutQtAct->setStatusTip(tr("Show the Qt library's About box"));
	connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	openHelpAct = new QAction(tr("H&elp"), this);
	openHelpAct->setStatusTip(tr("Show help file"));
	connect(openHelpAct, SIGNAL(triggered()), this,  SLOT(openHelp()));

	//-----------------------доступно только локально--------------------------------------------

	searchAct= new QAction(QIcon(":/pic/resource/pic/search.png"),tr("&Search"), this);
	searchAct->setStatusTip(tr("Search devices"));
	connect(searchAct, SIGNAL(triggered()), this, SLOT(searchDev()));
	searchAct->setEnabled(false);

	updateAct= new QAction(QIcon(":/pic/resource/pic/reload.png"),tr("&Status"), this);
	updateAct->setStatusTip(tr("Get status devices"));
	connect(updateAct, SIGNAL(triggered()), this, SLOT(updateDev()));
	updateAct->setEnabled(false);

	genSettings = new QAction(QIcon(":/pic/resource/pic/leftright.png"),tr("&Generate settings"), this);
	genSettings->setStatusTip(tr("Generation settings based on the list of search devices"));
	connect(genSettings, SIGNAL(triggered()), this, SLOT(GenSetting()));
	genSettings->setEnabled(false);

	//------------------------------прибор-канал---------------------------------------
/*
	infoAct = new QAction(QIcon(":/pic/resource/pic/info.png"),tr("&Info"), this);
	infoAct->setStatusTip(tr("Detailed information about the devices"));
	infoAct->setEnabled(false);
	connect(infoAct, SIGNAL(triggered()), this, SLOT(infoDev()));


	dataAct = new QAction(QIcon(":/pic/resource/pic/import.png"),tr("&Data"), this);
	dataAct->setStatusTip(tr("Get real-time data"));
	dataAct->setEnabled(false);
	connect(dataAct, SIGNAL(triggered()), this, SLOT(dataDev()));
*/

	//---------------------------------------------------------------------------------


	startAct = new QAction(QIcon(":/pic/resource/pic/question.png"),tr("&Start/Stop"), this);
	startAct->setStatusTip(tr("Not connected server."));
	startAct->setEnabled(false);
	connect(startAct, SIGNAL(triggered()), this, SLOT(StartStopServer()));

	//connect(SRV, SIGNAL(status(bool)), this, SLOT(setStatusSRV(bool)));

	connectAct= new QAction(QIcon(":/pic/resource/pic/disconnect.png"),tr("&Connect"), this);
	connectAct->setStatusTip(tr("Connecting to server"));
	connectAct->setEnabled(true);
	connect(connectAct, SIGNAL(triggered()), this, SLOT(openConnectToServer()));


	//testAct = new QAction(tr("&TEST"), this);
	//connect(testAct, SIGNAL(triggered()), this, SLOT(startOSC()));
}

void dgvector::createMenu(){

	fileMenu=menuBar()->addMenu(tr("&File"));

    fileMenu->addSeparator();
    fileMenu->addSeparator();
    fileMenu->addAction(quitAct);

    rtMenu=menuBar()->addMenu(tr("&Server"));
//    rtMenu->addAction(dataAct);
    rtMenu->addAction(startAct);
    rtMenu->addSeparator();
    rtMenu->addAction(connectAct);

    deviseMenu=menuBar()->addMenu(tr("&Device"));
    deviseMenu->addAction(searchAct);
    deviseMenu->addAction(updateAct);
    deviseMenu->addAction(genSettings);
//    deviseMenu->addAction(infoAct);




    viewMenu = menuBar()->addMenu(tr("&View"));


    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(openHelpAct);
    helpMenu->addAction(aboutAct);
    helpMenu->addSeparator();
    helpMenu->addAction(aboutQtAct);
}

void dgvector::createToolBar(){

	 toolBar = addToolBar(tr("Summary"));
	 toolBar->addSeparator();
//	 toolBar->addAction(searchAct);
	// toolBar->addAction(updateAct);
//	 toolBar->addAction(infoAct);
	// toolBar->addSeparator();
//	 toolBar->addAction(dataAct);

	// toolBar->addAction(genSettings);
	 toolBar->addAction(startAct);
	 toolBar->addSeparator();
	 toolBar->addAction(connectAct);
}


void dgvector::createStatusBar(){

	statusBar()->showMessage(tr("Connected to %1").arg(ServerHost+"..."));
}

void dgvector::createDockWindows(){

	//QString fileName;

	dock1 = new QDockWidget(tr("Device"), this);
	dock1->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

//	DevEditor = new deditor(XSD::DevXSD,XSD::createFile(XSD::appPath+"/"+XSD::DevXML),this);
	DevEditor = new deditor(XSD::DevXSD);


	DevEditor->setPolicyGetAttribute(true);//какие атрибуты добавлять - только обязательные

	DevEditor->setEditableFromMenu(false);  //редактор в меню вырубаем кроме см ниже
	DevEditor->EdContextMenu["devices"]=deditor::ClearNode; //можно удалять все
	DevEditor->EdContextMenu["node"]=deditor::DeleteNode;   //можно удалять блоки
//	DevEditor->ExtActionTree["node"]<<infoAct;    //открыть файл конф. прибора

	DevEditor->setEditableAllAttribute(false);//редактировать атрибуты нельзя кроме...
	DevEditor->setEdAttribute("alias");

	//замены названий узлов в дереве на названия атрибутов
	DevEditor->setMask("","alias,name");
	DevEditor->setMask("node","alias,id");
	//картинки
	DevEditor->setIdAttr("id");
	DevEditor->setIcons("devices",":/pic/resource/pic/network_24.png");
	DevEditor->setIcons("node",":/pic/resource/pic/vector.png",":/pic/resource/pic/vector_dis.png",":/pic/resource/pic/vector_en.png");


	dock1->setWidget(DevEditor);


	addDockWidget(Qt::LeftDockWidgetArea, dock1);
	viewMenu->addAction(dock1->toggleViewAction());
	if(Debug) connect(DevEditor,SIGNAL(sendMess(QString)),this,SLOT(debug(QString)));//отладка
	connect(DevEditor, SIGNAL(changeData()), DevEditor, SLOT(saveDom()));
	connect(DevEditor, SIGNAL(changeCurNode(QDomNode)),this,SLOT(changeNodeDev(QDomNode)));
	connect(dock1, SIGNAL(visibilityChanged (bool)),this,SLOT(visibleDev(bool)));
	//connect(DevEditor, SIGNAL(isSave()),this,SLOT(GenSetting()));


	dock1->setEnabled(false);

	//----------------------------------------------------------------------------

	dock2 = new QDockWidget(tr("Modbus setup"), this);
	dock2->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    //MdbEditor = new deditor(XSD::ModbusXSD,XSD::createFile(XSD::appPath+"/"+XSD::ModbusXML), this);
	MdbEditor = new deditor(XSD::ModbusXSD);
	MdbEditor->setPolicyGetAttribute(true);//какие атрибуты добавлять -
	MdbEditor->setNoEdAttribute("s1,s2,s3,s4,s5,s6"); //не редактируем

	MdbEditor->setEditableFromMenu(true); //редактор в меню
	MdbEditor->NoEdContextMenu["modbus"]=deditor::ClearNode;
	for(int i=0;i<mdata::snkan.count();i++){
		MdbEditor->NoEdContextMenu[mdata::snkan.at(i)]=deditor::DeleteNode;
	}

	MdbEditor->setMask("data","name,id");


	dock2->setWidget(MdbEditor);
	addDockWidget(Qt::LeftDockWidgetArea, dock2);
	viewMenu->addAction(dock2->toggleViewAction());
	//connect(saveAct, SIGNAL(triggered()), MdbEditor, SLOT(saveDom()));
	connect(MdbEditor, SIGNAL(changeData()), MdbEditor, SLOT(saveDom()));
	if(Debug) connect(MdbEditor, SIGNAL(sendMess(QString)), this, SLOT(debug(QString)));//отладка


	//-----------------------------------------------------------------------------
	dock3 = new QDockWidget(tr("Setup"), this);
		dock3->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

	//	SetEditor = new deditor(XSD::SettingsXSD,XSD::createFile(XSD::appPath+"/"+XSD::SettingsXML), this);
		SetEditor = new deditor(XSD::SettingsXSD);
		SetEditor ->setPolicyGetAttribute(true);//какие атрибуты добавлять -
		SetEditor->setEditableFromMenu(true); //редактор в меню
		//SetEditor->setMask("data","name,id");
		SetEditor->addNoEdAttribute("id");
		SetEditor->addNoEdAttribute("stype");
		SetEditor->addNoEdAttribute("band");
		SetEditor->addNoEdAttribute("resolution");

		SetEditor->setMask("modbus","modbus,alias");
		SetEditor->setMask("UDP","modbus,alias");
		SetEditor->setMask("COM485","comAddr");
		SetEditor->setMask("TCP","ipaddr");
		SetEditor->setMask("job","label");
		SetEditor->setMask("diap","label,band");

		//картинки
		SetEditor->setIdAttr("id");
		SetEditor->setIcons("settings",":/pic/resource/pic/setting.png");
		SetEditor->setIcons("COM485",":/pic/resource/pic/connect.png");
		SetEditor->setIcons("TCP",":/pic/resource/pic/network_24.png");
		SetEditor->setIcons("UDP",":/pic/resource/pic/vector.png",":/pic/resource/pic/vector_wait.png",":/pic/resource/pic/vector_con.png");
		SetEditor->setIcons("modbus",":/pic/resource/pic/vector.png",":/pic/resource/pic/vector_wait.png",":/pic/resource/pic/vector_con.png");
		SetEditor->setIcons("job",":/pic/resource/pic/xclock.png",":/pic/resource/pic/xclock_dis.png",":/pic/resource/pic/xclock_run.png");
		SetEditor->setIcons("osc",":/pic/resource/pic/curve.png");
		SetEditor->setIcons("client",":/pic/resource/pic/leftright.png");
		SetEditor->setIcons("server",":/pic/resource/pic/connect.png");
		SetEditor->setIcons("dbsrv",":/pic/resource/pic/database_save.png",":/pic/resource/pic/database_error.png",":/pic/resource/pic/database_go.png");
		SetEditor->setIcons("sdiag",":/pic/resource/pic/spectr.png");
		SetEditor->setIcons("opc_ua",":/pic/resource/pic/go-previous.png");
		SetEditor->setIcons("opcuser",":/pic/resource/pic/user.png");

		dock3->setWidget(SetEditor);
		addDockWidget(Qt::LeftDockWidgetArea, dock3);
		viewMenu->addAction(dock3->toggleViewAction());
		//connect(saveAct, SIGNAL(triggered()), MdbEditor, SLOT(saveDom()));
		connect(SetEditor, SIGNAL(AddManualNode(QDomElement)), this, SLOT(AddManualNode(QDomElement)));
		connect(SetEditor, SIGNAL(changeData()), SetEditor, SLOT(saveDom()));
		connect(SetEditor, SIGNAL(AttManualChange(QDomNode)), this, SLOT(setAttM(QDomNode)));

		if(Debug) connect(SetEditor, SIGNAL(sendMess(QString)), this, SLOT(debug(QString)));//отладка
	//-----------------------------------------------------------------------------

		dock4 = new QDockWidget(tr("Log"), this);
		//dock4->setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);
		LogEdit=new QPlainTextEdit(this);
		LogEdit->setReadOnly(true);
		LogEdit->setMaximumBlockCount(100);
		dock4->setWidget(LogEdit);
		addDockWidget(Qt::LeftDockWidgetArea, dock4);
		viewMenu->addAction(dock4->toggleViewAction());
		connect(logO::getInstance(), SIGNAL(logText(QString)), this, SLOT(appendLogTxt(QString)));


	//---------------------------------------------------------------------------
	tabifyDockWidget(dock3,dock4);
	tabifyDockWidget(dock3,dock2);
	tabifyDockWidget(dock3,dock1);


	dock1->setVisible(false);
	dock2->setVisible(false);
//	dock3->setVisible(true);
//	dock4->setVisible(true);

	dock3->raise();

}

//при изменении списка устройств
void dgvector::GenSetting(){
	xmlConverter xmlC(SetEditor);
	xmlC.convert(DevEditor->document);
}

void dgvector::about()
{
   QMessageBox::about(this, tr("About DGVector"),
		   tr("Gateway data collection devices")+
		   "\n\nDGVector ver. "+ver+"\n\n"+
		   tr("Development")+": '"+tr("GK Innovation")+"'\n\n"+
		   tr("Website")+ ": http://www.gkin.ru");
}

void dgvector::debug(QString mes){
	qDebug()<<mes;
}



/*
 * Показывает конфиг
 */
void dgvector::infoDev(){
	QDomNode    node=DevEditor->getCurDomNode();
	if(node.nodeName()!="node"){
		if(node.parentNode().nodeName()=="node"){
			 node=node.parentNode();
		}else return;
	}
	if(node.attributes().contains("id")){
		QString sn=node.attributes().namedItem("id").nodeValue();

		if(informer==NULL) informer= new info(":/templ/resource/template.html");
		informer->rep->setWindowTitle(sn);
		QString fn=XSD::appPath+"/CFG/"+sn+"/CFG.TXT";
		if(QFile::exists(fn))       informer->ViewGFG(fn);
	}
}


/*
 * Слот - поиск устройств
 */
void dgvector::searchDev(){
	dock1->setVisible(true);
	dock1->raise();
	createSearch();
	searchDevices->show();
}

/*
 * Слот - обновление устройств
 */
void dgvector::updateDev(){
	createSearch();
	searchDevices->close();
	genSettings->setEnabled(false);
	updateAct->setEnabled(false);
	//сечем время поиска
	searchDevices->getInfo();
}


void dgvector::createSearch(){
	if(searchDevices==NULL){
			searchDevices=new search(DevEditor,this);
			searchDevices->setWindowFlags(Qt::Dialog);
			searchDevices->setModal(true);
			searchDevices->setMaximumSize(QSize(250, 122));
			searchDevices->setWindowTitle(tr("Search devices"));
			connect(searchDevices, SIGNAL(Done()),DevEditor,SLOT(saveDom()));
			connect(searchDevices, SIGNAL(Done()),this,SLOT(endSearch()));
			connect(searchDevices, SIGNAL(Run()),this,SLOT(startSearch()));
			connect(searchDevices, SIGNAL(rejected()),searchDevices,SLOT(stop()));
		}
}

void dgvector::visibleDev(bool viz){


	if(!viz || !dock1->isEnabled()){
		searchAct->setEnabled(false);
		updateAct->setEnabled(false);
		genSettings->setEnabled(false);
	}else{
		genSettings->setEnabled(isRun==0);
		updateAct->setEnabled(isRun==0);
		searchAct->setEnabled(isRun==0);
		changeNodeDev(DevEditor->getCurDomNode());
	}

}


/*
 * Слот - конец поиска
 */
void dgvector::endSearch(){
	 searchAct->setEnabled(true);
	 updateAct->setEnabled(true);
	 genSettings->setEnabled(isRun==0);

	 if(VarningUpdate!=NULL){
		 VarningUpdate->close();
		 delete(VarningUpdate);
		 VarningUpdate=NULL;
	 }
}
/*
 * Слот - конец поиска
 */
void dgvector::startSearch(){
	 searchAct->setEnabled(false);
	 updateAct->setEnabled(false);
	 genSettings->setEnabled(false);
}

void dgvector::StartStopServer(){

	setBlockAction();

	startAct->setIcon(QIcon(":/pic/resource/pic/question.png"));
	if(isRun){
		timerState.stop();
		if(isRun==1) isRun=-2;
		getStatThr=rpcXmlCl->call("server.stop",QVariantList(),
				this,SLOT(serverStopped()),
				this,SLOT(maiaERR(int, const QString &)),3000,getStatThr);

	}else{

		runStd=0;
		timerState.stop();

		setConfThr=rpcXmlCl->call("server.setSettings",QVariantList()<<SetEditor->document.toByteArray(),
					this,SLOT(okSettings(const QVariant &)),
					this,SLOT(maiaERR(int, const QString &)),3000);

		setConfThr=rpcXmlCl->call("server.setModbusSett",QVariantList()<<MdbEditor->document.toByteArray(),
					this,SLOT(okModbusSett(const QVariant &)),
					this,SLOT(maiaERR(int, const QString &)),3000,setConfThr);

		//rpcXmlCl->close(setConfThr);

	}
}


void dgvector::openConnectToServer(){
	if(!tuner){
		tuner=new tunerWin(this);
		connect(tuner, SIGNAL(start(QString)), this, SLOT(connectToServer(QString)));
	}

	tuner->setAddres(Url.host());
	tuner->setStatusConn(isRun>=0);
	tuner->show();
}

void dgvector::okSettings(const QVariant& rez){
	if(rez.toBool()) runStd++;
	else{
		qWarning()<<"Not set settings config";
		getStatusRPC();
	}
}
void dgvector::okModbusSett(const QVariant& rez){

	if(rez.toBool()) runStd++;
	else{
		qWarning()<<"Not set modbus config";
		getStatusRPC();
	}
	if(runStd==2){

		setConfThr=rpcXmlCl->call("server.start",QVariantList(),
		this,SLOT(serverStopped()),
		this,SLOT(maiaERR(int, const QString &)),3000);
	}
}

void dgvector::serverStopped(){

	if(!runEv) QCoreApplication::quit();
	else {
		if(isRun==-2) QTimer::singleShot(1000, this, SLOT(getStatusRPC()));
		else          QTimer::singleShot(4000, this, SLOT(getStatusRPC()));
	}
}

void dgvector::setStatusSRV(const QVariant& run){

	bool stat=run.toBool();

//	qDebug()<<QTime::currentTime()<<"setStatusSRV:"<<stat<<isRun;


	timerState.stop();
    if(stat==true && isRun==-2){
		QTimer::singleShot(1000, this, SLOT(getStatusRPC()));
		return;
	}

	int pool=0;
	if(stat) pool=2000;
	else 	 pool=5000;

	if(isRun==-1){
		ServerHost=Url.host();
		statusBar()->showMessage(tr("Connect %1").arg(ServerHost));
		setWindowTitle(prName+tr(" (Connected to %1)").arg(ServerHost));
		getStatThr=rpcXmlCl->call("server.getLogTxt",QVariantList()<<logDT,
			 	 		this,SLOT(appendLogTxt(const QVariant &)),
			 	 		this,SLOT(maiaERR(int, const QString &)),3000,getStatThr);
		//запрос конфига
		rpcXmlCl->call("server.getSettings",QVariantList(),
						this,SLOT(appendSettings(const QVariant &)),
						this,SLOT(maiaERR(int, const QString &)));

		//запрос конфига модбаса
		rpcXmlCl->call("server.getModbusSett",QVariantList(),
						this,SLOT(appendModbusSett(const QVariant &)),
						this,SLOT(maiaERR(int, const QString &)));

		 dock1->setEnabled(ServerHost.toLower()=="localhost" || ServerHost.toLower()=="127.0.0.1");
		 if(tuner){
			 tuner->hide();
			 isParent=tuner->getParent();
		 }else isParent=0;
		 settings->setValue("/settings/ServerHost",ServerHost); //потом
		 connectAct->setIcon(QIcon(":/pic/resource/pic/connect.png"));
		 pool=1000;
	}else{

			int lt=timerState.interval()+1000;
			getStatThr=rpcXmlCl->call("server.getStatusV",QVariantList(),
						this,SLOT(setStatusDev(const QVariant &)),
						this,SLOT(maiaERR(int, const QString &)),lt,getStatThr);

			getStatThr=rpcXmlCl->call("server.getStatusOSCColV",QVariantList(),
						this,SLOT(setStatusOSC(const QVariant &)),
						this,SLOT(maiaERR(int, const QString &)),lt,getStatThr);

			getStatThr=rpcXmlCl->call("server.getStatusDB",QVariantList(),
						this,SLOT(setStatusDB(const QVariant &)),
						this,SLOT(maiaERR(int, const QString &)),lt,getStatThr);
	}

    if(isRun!=(int)stat){

		if (stat) {
			startAct->setIcon(QIcon(":/pic/resource/pic/pause.png"));
			startAct->setStatusTip(tr("Stop server."));
			trayIcon -> setIcon(QIcon(":/pic/resource/pic/vector_con.png"));
		} else {
			setDefStatusAction();
		}
    }else{
    	if(stat==false && !startAct->isEnabled()) setDefStatusAction();
    }

	isRun=(int)stat;
	if(!startAct->isEnabled())    startAct->setEnabled(true);
	if(!connectAct->isEnabled())  connectAct->setEnabled(true);
	visibleDev(dock1->isVisible());
	//genSettings->setEnabled(isRun==0 && dock1->isActiveWindow());
	timerState.start(pool);
}

void dgvector::setBlockAction(){
	startAct->setEnabled(false);
	connectAct->setEnabled(false);
	genSettings->setEnabled(false);
	SetEditor->setEditableFromMenu(false);
	SetEditor->setEditableAllAttribute(false);
	MdbEditor->setEditableFromMenu(false); //редактор в меню
	MdbEditor->setEditableAllAttribute(false);
}

void dgvector::setDefStatusAction(){
	startAct->setIcon(QIcon(":/pic/resource/pic/play.png"));
	startAct->setStatusTip(tr("Start server."));
	SetEditor->setEditableFromMenu(true);
	SetEditor->setEditableAllAttribute(true);
	trayIcon -> setIcon(QIcon(":/pic/resource/pic/vector.png"));
	MdbEditor->setEditableFromMenu(true); //редактор в меню
	MdbEditor->setEditableAllAttribute(true);
}

void dgvector::getStatusRPC(){

//	qDebug()<<QTime::currentTime()<<"getStatusRPC:"<<isRun<<(int*)getStatThr;

	int lt=timerState.interval()+1000;


	getStatThr=rpcXmlCl->call("server.isRun",QVariantList(),
				this,SLOT(setStatusSRV(const QVariant&)),
				this,SLOT(maiaERR(int, const QString &)),lt,getStatThr);

	if(isRun!=-1){
		getStatThr=rpcXmlCl->call("server.getLogTxt",QVariantList()<<logDT,
					this,SLOT(appendLogTxt(const QVariant &)),
					this,SLOT(maiaERR(int, const QString &)),lt,getStatThr);
    }


}

void dgvector::maiaERR(int err, const QString & txt){
	qWarning()<<err<<"-RPC Error:"+txt;
	if(err==-32500){
		statusBar()->showMessage(tr("Error connected to %1").arg(Url.host()));
		setWindowTitle(prName+tr(" (Disconnected)"));
		isParent=-1;
		timerState.stop();
		if(isRun!=-1){
			closeConnectToServer();
		}else{
			openConnectToServer();
		}
	}
}

void dgvector::closeConnectToServer(){
	setStatusSRV(QVariant(false));
	isRun=-1;
	timerState.stop();
	startAct->setEnabled(false);
	SetEditor->setDocument();
	MdbEditor->setDocument();
	DevEditor->setDocument();
	SetEditor->setEditableFromMenu(false);
	SetEditor->setEditableAllAttribute(false);
	LogEdit->clear();

	dock1->setVisible(false);
	startAct->setIcon(QIcon(":/pic/resource/pic/question.png"));
	startAct->setStatusTip(tr("Not connected server."));
	connectAct->setIcon(QIcon(":/pic/resource/pic/disconnect.png"));
	trayIcon->setIcon(QIcon(":/pic/resource/pic/vector_disconnect.png"));

}
/*
void dgvector::connectERR(int err, const QString & txt){
	qWarning()<<err<<"-RPC Error:"+txt;
	statusBar()->showMessage(tr("Error connected to %1").arg(ServerHost.toString()));
}
*/

void dgvector::setStatusDev(const QVariant& s){
	bool isSet=false;
	QVariantMap st=s.toMap();
	QVariantMap::const_iterator it=st.begin();


	for(;it!=st.end();it++){
		switch(it.value().toUInt()){
			case 2:
				if(SetEditor->getIdStatus(it.key())!=1){
					SetEditor->setIdStatus(it.key(),1);
					isSet=true;
				}
				break;
			case 1:
				if(SetEditor->getIdStatus(it.key())!=-1){
					SetEditor->setIdStatus(it.key(),-1);
					isSet=true;
				}
				break;
			case 0:
				if(SetEditor->getIdStatus(it.key())!=0){
					SetEditor->setIdStatus(it.key(),0);
					isSet=true;
				}
				break;
		}
	}

	if(isSet){
		QModelIndex is=SetEditor->indexTreeFromNode(SetEditor->rootNode);
		SetEditor->ui.treeView->collapse(is);
		SetEditor->ui.treeView->expand(is);
	}
}

void dgvector::setStatusOSC(const QVariant& s){

	int st;
	bool isSet=false;
    QVariantList ShedList=s.toList();
	for(int i=0;i<ShedList.size(); i++){
		st=ShedList.at(i).toInt();
		QString id("osc,job,"+QString::number(i));
		if(SetEditor->getIdStatus(id)!=st){
			SetEditor->setIdStatus(id,st);
			isSet=true;
		}
	}
	if(isSet){
		QModelIndex is=SetEditor->indexTreeFromNode(SetEditor->rootNode);
		SetEditor->ui.treeView->collapse(is);
		SetEditor->ui.treeView->expand(is);
	}

}

void dgvector::setStatusDB (const QVariant& s){

	int st=s.toInt();
	QString id("settings,dbsrv,"+QString::number(SetEditor->rootNode.childNodes().size()-1));

	if(SetEditor->getIdStatus(id)!=st){
		SetEditor->setIdStatus(id,st);
		QModelIndex is=SetEditor->indexTreeFromNode(SetEditor->rootNode);
		SetEditor->ui.treeView->collapse(is);
		SetEditor->ui.treeView->expand(is);
	}


}

void dgvector::appendLogTxt(const QVariant& s){
	 QStringList logMes=s.toStringList();
	 for(int i=0;i<logMes.size();i++){
		  logDT=QDateTime::fromString(logMes[i].left(21),"yyyyMMddThh:mm:ss.zzz");
	      LogEdit->appendPlainText(logMes[i]);//отладка
	 }
}

void dgvector::appendLogTxt(QString s){
	 LogEdit->appendPlainText(s);//отладка
}

void dgvector::appendSettings(const QVariant& s){
	QDomDocument doc;
	QString errMsg;
	int errorLine;
	int errorColumn;
	QString dir=XSD::appPath+"/tmp";
	QString xmlName(dir+"/"+XSD::SettingsXML);
	QDir().mkpath(dir);
	//qDebug()<<"!!"<<QString(s.toByteArray());
	doc.setContent(s.toByteArray(),&errMsg,&errorLine,&errorColumn);
	if(errMsg!=""){
		qWarning()<<tr("Error (settings): %1. Line:%2, Column:%3").arg(errMsg).arg(errorLine).arg(errorColumn);
		XSD::createFile(xmlName);
	}else{
		QFile file(xmlName);
		if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
			qDebug() <<"Not open write file:"+xmlName;
			return;
		}
		QTextStream out(&file);
		out.setCodec("UTF-8");
		out<<doc.toString();
		file.close();
	}
	SetEditor->setDocument(xmlName);
}

void dgvector::appendModbusSett(const QVariant& s){
	QDomDocument doc;
	QString errMsg;
	int errorLine;
	int errorColumn;
	QString dir=XSD::appPath+"/tmp";
	QString xmlName(dir+"/"+XSD::ModbusXML);
	QDir().mkpath(dir);

	doc.setContent(s.toByteArray(),&errMsg,&errorLine,&errorColumn);
	if(errMsg!=""){
		qWarning()<<tr("Error (modbus): %1. Line:%2, Column:%3").arg(errMsg).arg(errorLine).arg(errorColumn);
		XSD::createFile(xmlName);
	}else{
		QFile file(xmlName);
		if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
			qDebug() <<"Not open write file:"+xmlName;
			return;
		}
		QTextStream out(&file);
		out.setCodec("UTF-8");
		out<<doc.toString();
		file.close();
	}
	MdbEditor->setDocument(xmlName);
}



void dgvector::messageBAD(const QString& name,const QByteArray& mess){
	QString terr;
	if(Debug) qDebug()<<"ERROR: "+name<<execut::ByteArray_to_String(mess);
}

void dgvector::changeNodeDev(QDomNode node){

	QString name=node.nodeName();
	tnode tipn=root;
	tcan  tipc=adc;
	if(name=="devices")          tipn=root;
	else if(name=="node")        tipn=device;
	else if(name.left(3)=="ADC") {tipn=canal;  tipc=adc;}
	else if(name.left(3)=="TAH") {tipn=canal;  tipc=tah;}
	else if(name.left(3)=="VIR") {tipn=canal;  tipc=vir;}


//	infoAct->setEnabled(false);
//	dataAct->setEnabled(false);

	switch(tipn){
		case root:
				break;
		case device:
//				infoAct->setEnabled(true);
//				dataAct->setEnabled(true && en);
				break;
		case canal:
//				infoAct->setEnabled(true);
//				dataAct->setEnabled(true && en);
				switch(tipc){
					case adc:
								//getRealOscAct->setEnabled(true && en);

								break;
					case tah:	break;
					case vir:	break;
				}

	}

}

void dgvector::openHelp(){


	   // format systems language
	    QString defaultLocale = QLocale::system().name();       // e.g. "de_DE"
	    defaultLocale.truncate(defaultLocale.lastIndexOf('_')); // e.g. "de"
	    QString hfile=QDir::currentPath()+"/help/"+defaultLocale+"/index.html";
	    QFileInfo finf(hfile);
	    if(finf.exists() && finf.isReadable()){
			helpBr=new helpBrowser(hfile,this);
			helpBr->setAttribute(Qt::WA_DeleteOnClose);
			helpBr->setWindowFlags(Qt::Dialog);
			helpBr->setWindowTitle(tr("Help"));
			helpBr->resize(700,500);
			helpBr->show();
	    }else{
	    	QMessageBox::warning(
				this,
				tr("Error"),
				tr("Help File not found!"),
				QMessageBox::Close,QMessageBox::Close);
	    }
}

/**
 * Закрытие программы
 */
void dgvector::closeEvent(QCloseEvent *event){

	if(!isVisible()) showNormal();
	if(QMessageBox::question(
					this,
					tr("Confirmation"),
					tr("Quit program?"),
					QMessageBox::Yes | QMessageBox::No,
					QMessageBox::Yes)==QMessageBox::Yes){

				mdiArea->closeAllSubWindows();
				if(isRun>0 && isParent==1){
					    runEv=false;
					    timerState.stop();
					    getStatThr=rpcXmlCl->call("server.stop",QVariantList(),
							this,SLOT(serverStopped()),
							this,SLOT(maiaERR(int, const QString &)),3000,getStatThr);
					    event->ignore();

				}else{
					event->accept();
					//return;
					//exit(0);
					QCoreApplication::quit();
				}
	}else{
		event->ignore();
	}
}



//Работа с треем
//***********************************************************************

void dgvector::showTrayIcon()
{
    // Создаём экземпляр класса и задаём его свойства...
    trayIcon = new QSystemTrayIcon(this);
    trayIcon -> setIcon(QIcon(":/pic/resource/pic/vector_disconnect.png"));
    trayIcon -> setContextMenu(trayIconMenu);

    // Подключаем обработчик клика по иконке...
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));

    // Выводим значок...
    trayIcon -> show();
}

void dgvector::trayActionExecute()
{
    //QMessageBox::information(this, "TrayIcon", tr("Test"));
    if(!isVisible()){
          showNormal();
    }else{
		hideTray();
	}
}

void dgvector::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
        case QSystemTrayIcon::Trigger:
        case QSystemTrayIcon::DoubleClick:
           trayActionExecute();
            break;
        default:
            break;
    }
}

void dgvector::setTrayIconActions()
{
    // Setting actions...
    minimizeAction = new QAction(tr("Collapse"), this);
    restoreAction = new QAction(tr("Restore"), this);

    // Connecting actions to slots...
    connect (minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));
    connect (restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));


    // Setting system tray's icon menu...
    trayIconMenu = new QMenu(this);
    trayIconMenu -> addAction (minimizeAction);
    trayIconMenu -> addAction (restoreAction);
    trayIconMenu -> addAction (quitAct);
}

void dgvector::hideTray(){
//	if(dataCan!=NULL)  if(dataCan->isVisible())  dataCan->close();
    if(informer!=NULL) if(informer->rep->isVisible()) informer->rep->close();
    hide();
}

void dgvector::changeEvent(QEvent *event)
{
    if (event -> type() == QEvent::WindowStateChange)
    {
        if (isMinimized()){
           // hideTray();
        	bool h=false;
        	if(!tuner)  h=true;
        	else        h=tuner->isHidden();
        	if(h) QTimer::singleShot(0, this, SLOT(hideTray()));
            event->ignore();
            return;
         }
    }
    QMainWindow::changeEvent(event);

}

void dgvector::AddManualNode(QDomElement e){
	if(e.nodeName()=="modbus" || e.nodeName()=="UDP"){
		if(e.attribute("id","")==""){
		     QString id; //=QString::number(qrand(),16)+QString::number(qrand(),16);
			 while(id.size()<16){
				 id.append(QString::number(qrand()*QDateTime::currentDateTime().currentSecsSinceEpoch(),16));
			 }
		     e.setAttribute("id",id.left(16));
		}
	}

	if(e.nodeName()=="A" || e.nodeName()=="C"){
		sdiagCollector::setAC(e);
		SetEditor->goElement(e);
	}
	if(e.nodeName()=="diap"){
		sdiagCollector::setDiap(e);
		SetEditor->goElement(e);
	}
}


void dgvector::setAttM(QDomNode node){
	bool ok;

	if(node.nodeName()=="diap"){
		sdiagCollector::setDiap(node);
		SetEditor->goElement(node.toElement());
	}

	if(node.nodeName()=="A" || node.nodeName()=="C"){
		sdiagCollector::setAC(node);
		SetEditor->goElement(node.toElement());
	}
}
