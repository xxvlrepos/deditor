/*
 * helpBrowser.h
 *
 *  Created on: 15.05.2013
 *      Author: Usach
 */

#ifndef HELPBROWSER_H_
#define HELPBROWSER_H_
#include <QWidget>
#include <QPushButton>
#include <QToolButton>
#include <QTextBrowser>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QAction>

#ifdef _WEBKIT_
#include <QWebView>
#include <QWebHistory>
#endif

class helpBrowser: public QWidget {

	Q_OBJECT

public:
	helpBrowser(QString File,QWidget *parent = 0);
	virtual ~helpBrowser();
	QUrl url;
#ifndef _WEBKIT_
	QTextBrowser* ptxtBrowser;
#else
	QWebView*     ptxtBrowser;
#endif

public slots:
 void home();
 void endLoad(bool);

};

#endif /* HELPBROWSER_H_ */
