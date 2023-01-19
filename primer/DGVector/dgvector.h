#ifndef DGVECTOR_H
#define DGVECTOR_H

//#include <server.h>
#include <QMainWindow>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QMdiArea>
#include <QDockWidget>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QSystemTrayIcon>

#include <const.h>
#include <mdata.h>
#include <deditor.h>
#include <search.h>
#include <info.h>
#include <helpBrowser.h>
#include "xmlConverter.h"
#include <mdata.h>
#include "./libmaia/maiaXmlRpcClient.h"
#include <tunerwin.h>



class logO: public QObject{

	Q_OBJECT

public:
	static logO* m_instance;

	logO():QObject(0){};
	static logO* getInstance(){
		 if (m_instance == 0)  m_instance = new logO;
		 return m_instance;
	}
#if QT_VERSION >= 0x050000
    static  void logOutput5(QtMsgType type,const QMessageLogContext &context, const QString& msg){
		logOutput(type,msg.toLocal8Bit().constData());
	}
#endif
	static  void logOutput(QtMsgType type, const char *msg)
	{
	    QDateTime curDT=QDateTime::currentDateTime();
	   	QString deb(curDT.toString("yyyyMMddThh:mm:ss.zzz"));
	       switch (type)
	       {
	   		case QtDebugMsg:    deb.append("[D] "); break;
	   		case QtWarningMsg:  deb.append("[W] "); break;
	   		case QtCriticalMsg: deb.append("[C] "); break;
	   		case QtFatalMsg:    deb.append("[F] "); break;
	       }
	       deb.append(msg);
	       getInstance()->emitSignal(deb);
	   }

signals:
	void logText(QString);

private:

    void emitSignal(QString d){
		emit logText(d);
	}
};




class dgvector : public QMainWindow
{
    Q_OBJECT

public:
    dgvector(QWidget *parent = 0);
    ~dgvector();

    static bool  Debug;

    QString				prName;		  //имя программы
    QString				ver;          //версия совта
    QSettings 			*settings;    //сохранение настроек в ini

    //содержимое вкладок
    deditor *DevEditor; //редактор устройств
    deditor *MdbEditor; //редактор регистров модбас
    deditor *SetEditor; //редактор настроек
    QDockWidget *dock1,*dock2,*dock3,*dock4;
    QPlainTextEdit   *LogEdit;

    QMdiArea      *mdiArea;
    QString       ServerHost;
    QUrl          Url;
   	//----------------------------------------
    enum tnode{	root=0,	device,	canal};
    enum tcan{adc=0,tah,vir	};
  //  static void logOutput(QtMsgType type, const char *msg);

signals:
    void logTxt(QString);

public slots:
	void closeEvent(QCloseEvent *event); //закрытие программы
//	void updateMenus();
	void messageBAD(const QString&,const QByteArray&);//слот приема сообщений для всплывающих окон
    void GenSetting();

private:

    bool runEv;
    QTimer                    timerState;

    QToolBar *toolBar;

    info         *informer;//показывает конфиг
 //   dataC        *dataCan; //получение даных из прибора по каналу
    search       *searchDevices;//поиск приборов
    helpBrowser  *helpBr;//просмотрщий хелпа
    tunerWin     *tuner; //Настройка соединения


    QMenu   *fileMenu,*deviseMenu,*rtMenu,*viewMenu,*helpMenu;
    QAction *quitAct;
    QAction *searchAct;
    QAction *updateAct;
    QAction *aboutAct;
    QAction *aboutQtAct;
//    QAction *infoAct;
//    QAction *dataAct;
    QAction *openHelpAct;
    //---------------------------
    QAction *genSettings;
    QAction *startAct;
    QAction *connectAct;
    //------------------
    QAction *testAct;
    QMessageBox *VarningUpdate;

    //сворачивание в трей
    QMenu *trayIconMenu;
    QAction *minimizeAction;
    QAction *restoreAction;
    QSystemTrayIcon *trayIcon;

    void showTrayIcon();
    void setTrayIconActions();

    //---------------------------

    void createActions();
    void createMenu();
    void createToolBar();
    void createStatusBar();
    void createDockWindows();
    void createSearch();

    void setDefStatusAction();
    void setBlockAction();


private slots:
	void about();
	void openHelp();
	void debug(QString);
	void infoDev();//инфо о конфиге
//	void dataDev();//получить данные
	void searchDev();     //Поиск устройств
	void updateDev();     //Обновить инф. о устройствах
	void startSearch();  //блокировки кнопок поиска
	void endSearch();    //разблокировки кнопок поиска

	void changeNodeDev(QDomNode);     //изменяем контекстное меню
	void AddManualNode(QDomElement);
	void setAttM(QDomNode);         //какой-то атрибут поменялся

	//void updateStatus();             //состояние опроса

	void StartStopServer();
	void openConnectToServer();
	void closeConnectToServer();
	void setStatusSRV(const QVariant&);
	void versionTest(const QVariant&);
	void serverStopped();
	void visibleDev(bool);
	void okSettings(const QVariant &);
	void okModbusSett(const QVariant &);


	void maiaERR(int,const QString &);
	void connectToServer(QString);
	void getStatusRPC();       //посылка запроса состояния по сети
	void setStatusDev(const QVariant&);
	void setStatusOSC(const QVariant&);
	void setStatusDB (const QVariant&);
	void appendLogTxt(const QVariant&);
	void appendLogTxt(QString);
	void appendSettings(const QVariant&);
	void appendModbusSett(const QVariant&);


	//работа в трее
	 void changeEvent(QEvent*);
	 void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
	 void trayActionExecute();
     void hideTray();

protected:

     MaiaXmlRpcClient* rpcXmlCl;
     clientThread    *getStatThr,*setConfThr;
     int               isRun;  //-2 отключается,-1-неизвестно,0-отключен,1-работает
     int               isParent;
     QDateTime         logDT;
     int runStd;//стадии запуска


     //-------------------------------
};

#endif // DGVECTOR_H
