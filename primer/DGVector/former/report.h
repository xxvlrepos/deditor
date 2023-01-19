#ifndef REPORT_H
#define REPORT_H

#include <QtCore>
#include <QDialog>
#include <QFileDialog>
#include <QPrintDialog>
#include <QPrinter>
#include "ui_report.h"

class report : public QDialog
{
    Q_OBJECT

public:
    report(QWidget *parent = 0, QString cat = "./",int cid=0); //второе - рабочий каталог
    ~report();

    void closeEvent(QCloseEvent *event); //закрытие окна

signals:
    void Close(int);

protected:

	int Id;//идентификатор окна

private:
    Ui::reportClass ui;
    //virtual void resizeEvent(QResizeEvent *event);
    //static const int bw=5; //отступы от края виджета
   //static const int bh=5;

    QUrl* 		 url;        	//открываемый файл
    QString*     str;			//открываемая строка

    QString 	fileSeparator;	//разделитель имени файла (платформозависимый)
    QString 	fcName;   		//для сохранения путей
    QPrinter*	printer;    	//Принтер


public slots:

	void OpenFile(); 		//прочитать файл
	void OpenString();	//читаем строку
	void SaveFileAs();  	//сохранить как
	void Print();			//Печать
	void Reload();			//перезагружаем строку или файл

public:
	void set_Url(QUrl*);		 //сменить файл
	void set_String(QString*); //сменить строку
};

#endif // REPORT_H
