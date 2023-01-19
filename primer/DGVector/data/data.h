#ifndef DATA_H
#define DATA_H

#include <QtGui/QDialog>
#include "ui_data.h"
#include <QDomDocument>
#include <QDomNode>
#include <QtCore>
#include "../execut/tape.h"
#include "../execut/execut.h"
#include "datatable.h"


//-------------------------------------------------------
class emiterD : public QObject{
	Q_OBJECT
public:
	signals:
	void send(quint32,const QByteArray&);
	void sendBAD(quint32);
public:
	void emitSignals(quint32 ip32, const QByteArray& rez){  emit send(ip32,rez); }
	void emitSignalsBAD(quint32 ip32){ emit sendBAD(ip32);	}
};



class dataC : public QDialog
{
    Q_OBJECT

public:

    dataC(QString registers, QWidget *parent = 0);
    ~dataC();
    static emiterD* Emit;
    static emiterD* getEmit(){
      	if (Emit == 0) 	Emit = new emiterD;
      	return Emit;
    }
    static void funcOK(quint32,const QByteArray&);
    static void funcBAD(quint32,const QByteArray&);
    bool setSrcData(QDomNode,uint per,int nu=1); //число узлов с таким адресом - по факту >1
    //соединение по TCP

    void closeEvent(QCloseEvent *event); //закрытие программы

signals:
   void Close();   //закрытие окна

public slots:
	void Stop();
	void setSrcData(QDomNode node) {setSrcData(node,period);}


private:
    Ui::dataClass* ui;
    QString fileName;
    QDomDocument document;
    QDomElement  rootNode;
    QList<QDomNode> list;
    QList<int>      divider;
    execut exe;
    DataTable  *DT;

    union ucs{
        	char ch[2];
        	short sh;
     };

    int period;
    int CountReg; //глобальный счетчик регистров
private slots:

  void updateDat(quint32,const QByteArray&);
  void badDat(quint32);

};


#endif // DATA_H
