#ifndef TUNERWIN_H
#define TUNERWIN_H

#include <QDialog>
#include "ui_tunerwin.h"
#include <QProcess>
#include <QPushButton>


class tunerWin : public QDialog
{
    Q_OBJECT

public:
    tunerWin(QWidget *parent = 0);
    ~tunerWin();

    QString nameProc;
    static QStringList DebARG;
    bool getParent();//есть ли у сервера родитель
    void setAddres(QString ad){ ui.lineEdit->setText(ad);}
    void setStatusConn(bool);
    void killProcess(){
    	ui.pushButtonCD->setChecked(false);
    	ui.checkBox->setChecked(false);
    }
    signals:
	   void start(QString);

private:
    Ui::tunerWinClass ui;
    QProcess *process;



private  slots:
    void createProcess(bool);
	void clickedButtCD(bool);
	void isFinished(int,QProcess::ExitStatus);
	void isStart();
	void isDelayStart();
	void isError(QProcess::ProcessError);
	void setCheckBox(bool);
    void readyReadStandardOutput();

};

#endif // TUNERWIN_H
