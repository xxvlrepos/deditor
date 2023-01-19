#include "report.h"
#include <QCloseEvent>

report::report(QWidget *parent, QString cat, int cid): QDialog(parent)
{
	#ifdef _WIN_
	   fileSeparator="/";
	#else
	   fileSeparator="/";
	#endif

	url=NULL;
	str=NULL;
	printer=NULL;

	Id=0;
	if(cid!=0) Id=cid;

	fcName=cat;
	ui.setupUi(this);



	//кнопка
	QObject::connect(ui.SaveAsButton, SIGNAL(clicked()), this ,SLOT(SaveFileAs()));
	QObject::connect(ui.PrintButton, SIGNAL(clicked()), this ,SLOT(Print()));

}

report::~report(){
	if(printer) delete printer;
}

/*
void report::resizeEvent(QResizeEvent*){
	int dw= width()-minimumWidth();
	int dh= height()-minimumHeight();
	//ui.verticalLayoutWidget->setGeometry(bw,bh,dw-bw*2,dh-bh*2);
}
*/

/**
 * Смена файла
 */
void report::set_Url(QUrl* ur){
	url=ur;

}

/**
 * Смена строки
 */
void report::set_String(QString* st){
	str=st;
}



/**
 * читаем файл
 */
void report::OpenFile(){
	ui.textBrowser->clear();
	if(url->toString().trimmed()!="") {
		ui.textBrowser->setSource(*url);
		ui.textBrowser->reload();
	}
}

/**
 * читаем строку
 */
void report::OpenString(){
	ui.textBrowser->clear();
	if(str->trimmed()!="") {
		ui.textBrowser->append(*str);
	}
}

/**
 *  Обновление
 */
void report::Reload(){
	if(url!=NULL){
		QFileInfo info(url->toString());
		if(info.isFile() && info.isReadable()) OpenFile();
	} else {
		if(str!=NULL) OpenString();
	}
}


/* Слот
 * Записываем файл как
 */
void report::SaveFileAs(){

	QFileInfo info(fcName);

	if(info.isDir()) fcName+=fileSeparator+"report.html";
	else {
		if(info.isFile() && info.suffix().toUpper()!="HTML") fcName+=".html";
	}
	fcName=QFileDialog::getSaveFileName(
			this,
			tr("Save as file"),
			fcName
			);

	if(!fcName.isEmpty()){ //выбрано
		if(fcName.indexOf(".html",-5,Qt::CaseInsensitive)<0) fcName+=".html";
		//qDebug()<<fcName;
		QFile file;
		file.setFileName(fcName);
		if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
		QTextStream out(&file);
		out.setCodec("UTF-8");
		out<<*str;
		file.close();
		//UnParseFile(fcName);
	}

}

/**
 * Печать на принтер
 */
void report::Print(){
	if(printer==NULL){
			printer = new QPrinter;
			printer->setPageSize(QPrinter::A4);
			printer->setOrientation(QPrinter::Landscape);
			printer->setPageMargins(15,15,15,15,QPrinter::Millimeter);
	}

	QPrintDialog dlg(printer,this);
	if(dlg.exec()== QDialog::Accepted){
        ui.textBrowser->print(printer);
	}

}

/*
*  Закрытие окна
*/
void report::closeEvent(QCloseEvent *event){
    	emit Close(Id);
    	event->accept();
}

