#include "tunerwin.h"
#include "dgvector.h"

QStringList tunerWin::DebARG("-e");

tunerWin::tunerWin(QWidget *parent) : QDialog(parent)
{
	ui.setupUi(this);
	connect(ui.checkBox, SIGNAL(toggled(bool)),this,SLOT(setCheckBox(bool)));
	connect(ui.pushButtonCD, SIGNAL(toggled(bool)),this,SLOT(clickedButtCD(bool)));

	process=NULL;

nameProc="dgserver";
#ifdef _WIN_
	nameProc+=".exe";
#else
	nameProc+="/"+nameProc;
#endif

	setWindowTitle(tr("Connect to server"));
}

tunerWin::~tunerWin()
{
	if(process){
		if(process->state()!=QProcess::NotRunning){
		    disconnect(process, SIGNAL(error(QProcess::ProcessError)),this,SLOT(isError(QProcess::ProcessError)));
			process->close();
			if(process->state()>0) process->kill();
		}
	}
}

void tunerWin::setCheckBox(bool ck){
	if(ck) ui.lineEdit->setText("localhost");
	else   ui.lineEdit->setText(((dgvector*)parentWidget())->ServerHost);
	ui.lineEdit->setDisabled(ck);
}

void tunerWin::createProcess(bool check){


	ui.checkBox->setDisabled(true);
	ui.pushButtonCD->setDisabled(true);

	if(check){
	//	ui.lineEdit->setText("localhost");
		if(!process){
			process=new QProcess(this);
			connect(process, SIGNAL(readyReadStandardOutput()),this,SLOT(readyReadStandardOutput()));
		    connect(process, SIGNAL(finished ( int, QProcess::ExitStatus)),this,SLOT(isFinished(int,QProcess::ExitStatus)));
		    connect(process, SIGNAL(started()),this,SLOT(isDelayStart()));

		}
	    connect(process, SIGNAL(error(QProcess::ProcessError)),this,SLOT(isError(QProcess::ProcessError)));
		process->start(QCoreApplication::applicationDirPath()+"/"+nameProc,tunerWin::DebARG);

	}else{
		if(process!=NULL){
			emit start("");
		    disconnect(process, SIGNAL(error(QProcess::ProcessError)),this,SLOT(isError(QProcess::ProcessError)));
			process->close();
			//if(process->state()>0) process->kill();
			return;
		}

	}
	ui.lineEdit->setDisabled(check);
	ui.checkBox->setDisabled(false);
	ui.pushButtonCD->setDisabled(false);

}

void tunerWin::isDelayStart(){

	ui.pushButtonCD->setDisabled(false);
	hide();
	QTimer::singleShot(500,this,SLOT(isStart()));
}

void tunerWin::isStart(){
	emit start(ui.lineEdit->text());
}

void tunerWin::isFinished(int kode,QProcess::ExitStatus stat){
	if(kode==2 && stat==QProcess::NormalExit){
		qDebug()<<tr("Error! Server running on localhost earlier!");
		ui.checkBox->setChecked(false);
		return;
	}
	setStatusConn(false);
	emit start("");
}

void tunerWin::isError(QProcess::ProcessError err){
	qDebug()<<"Error"<<err<<process->errorString();
}

void tunerWin::clickedButtCD(bool con){

	 ui.pushButtonCD->setEnabled(false);
	 if(con){
		 if(ui.checkBox->isChecked()) createProcess(true);
		 else {
			 ui.lineEdit->setEnabled(false);
			 ui.checkBox->setEnabled(false);
			 emit start(ui.lineEdit->text());
		 }
	 }else{
		 if(ui.checkBox->isChecked()) createProcess(false);
		 else{
			 emit start("");
			 ui.pushButtonCD->setText(tr("Connect"));
			 ui.checkBox->setEnabled(true);
			 ui.lineEdit->setEnabled(true);
		 }
	 }
	 ui.pushButtonCD->setEnabled(true);

}

void tunerWin::setStatusConn(bool con){

	disconnect(ui.pushButtonCD, SIGNAL(toggled(bool)),this,SLOT(clickedButtCD(bool)));
	ui.pushButtonCD->setChecked(con);
	ui.lineEdit->setEnabled(!con);
	ui.checkBox->setEnabled(!con);
	if(!con){
		 ui.pushButtonCD->setText(tr("Connect"));
	}else{
		 ui.pushButtonCD->setText(tr("Disconnect"));
	}
	connect(ui.pushButtonCD, SIGNAL(toggled(bool)),this,SLOT(clickedButtCD(bool)));

}

bool tunerWin::getParent(){
   	if(!process) return 0;
   	return process->state()!=QProcess::NotRunning;
}



void tunerWin::readyReadStandardOutput(){
	while(process->canReadLine()){
	       qDebug() << process->readLine();
	}
}
