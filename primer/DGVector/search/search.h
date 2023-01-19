#ifndef SEARCH_H
#define SEARCH_H

#include <QDialog>
#include "ui_search.h"
#include <const.h>
#include <execut.h>
#include "../execut/tape.h"
#include "../deditor/deditor.h"
#include "../parser/parser.h"


//----------------------------------------------------------

/*
class emiterS : public QObject{
      	 Q_OBJECT
    public:
      	void addSearchNode(quint32 ip,const QByteArray& res){ emit addSearchNodeS(ip,res);}
      	void addVersionNode(const QString& t,const QByteArray& s,const QByteArray& r){
      		emit addVersionNodeS(t,s,r);
      	}
   	signals:
      	void addSearchNodeS(quint32,const QByteArray&);
      	void addVersionNodeS(const QString&,const QByteArray&,const QByteArray&);
};
*/

class search : public QDialog
{
    Q_OBJECT

public:

    search(deditor*,QWidget *parent = 0);
    ~search();

    static const unsigned short udpPort=502;
    deditor* editor; //кем рулим


    /*
    static emiterS* Emit;
    static emiterS* getEmit(){
    	if (Emit == 0) 	Emit = new emiterS;
    	return Emit;
    }
*/

    void getInfo(); //обновить информацию в документе о всех устройствах из имеющегося списка (полная выкачка конфигов и тд)


    /*
   static void funcOK(quint32 ip,const QByteArray& res) {
    	getEmit()->addSearchNode(ip,res);    }

   static void funcREZ(quint32 ip,const QByteArray& res){
    	getEmit()->addSearchNode(ip,res);
    }
    static void updateDataS(const QString& t,const QByteArray& s,const QByteArray& r){
    	getEmit()->addVersionNode(t,s,r);
    }
*/

    QString OldFile; //имя старого файла

    signals:
    void Run();
    void Done();
    void addSearchNode(quint32,const QByteArray&);

public slots:
       void stop();


private:

    static search* m_instance;

    Ui::searchClass ui;
    QStringList listCOM; //список портов
    QStringList BaudRate;
    bool Broadcast;
    bool isCOM;
    QMap<QString,int> Masters; //словарь модбас-мастеров (ip adrec, modbus adres)
   	parser	arrs;	   			//разобраный словарь строк файла - параметры в верхнем регистре
   struct dovn{
	   QString ge;
	   QStringList lut;
   };




   enum stg{Search=0,SearchSlave,GetInfo,ParseInfo_1,ParseInfo_2};
   stg Stage;//этап поиска

   execut exe;
   void interfaceEnable(bool);
   void searchCOM();
   void searchLAN();
   void addNode();
   void parseInfo_1(); //обновить луты и ge по имеющемуся конфигу, а затем и сам документ
   void parseInfo_2(); //обновить документ по конфигу
   void start2();//второй этап поиска по по модбас-slave

   int testIP(QString ip);//число блоков с таким IP (допустимость соединения по UDP)
   int testID(QString id);//включен, нет, неизвестно
   void setOK(QString);
   void setBAD(QString);
   void addVersion(const QString&,const QByteArray&,const QByteArray&);
   void addNode(quint32,const QByteArray&);

private slots:
	void setInterfaseToCOM(bool);
	void start(bool);
	void stop(bool);
	void setBroadcast(bool);
	void headerRez(command*);

};


#endif // SEARCH_H


