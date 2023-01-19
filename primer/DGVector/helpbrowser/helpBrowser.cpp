/*
 * helpBrowser.cpp
 *
 *  Created on: 15.05.2013
 *      Author: Usach
 */

#include "helpBrowser.h"

helpBrowser::helpBrowser(QString File,QWidget *parent)  : QWidget(parent)
{
	url=QUrl::fromLocalFile(File);

	QToolButton*  pcmdBack = new QToolButton();
	pcmdBack->setText("<<");
	QToolButton*  pcmdHome = new QToolButton();
	pcmdHome->setText(tr("Home"));
	QToolButton*  pcmdForward = new QToolButton();
	pcmdForward->setText(">>");

#ifndef _WEBKIT_

	ptxtBrowser = new QTextBrowser(this);

	connect(pcmdBack,SIGNAL(clicked()),ptxtBrowser,SLOT(backward()));
	connect(pcmdHome,SIGNAL(clicked()),ptxtBrowser,SLOT(home()));
	connect(pcmdForward,SIGNAL(clicked()),ptxtBrowser,SLOT(forward()));
	connect(ptxtBrowser,SIGNAL(backwardAvailable(bool)),pcmdBack,SLOT(setEnabled(bool)));
	connect(ptxtBrowser,SIGNAL(forwardAvailable(bool)),pcmdForward,SLOT(setEnabled(bool)));
	ptxtBrowser->setSource(url);

#else

	ptxtBrowser = new QWebView(this);
	pcmdBack->setDefaultAction(ptxtBrowser->pageAction(QWebPage::Back));
	pcmdForward->setDefaultAction(ptxtBrowser->pageAction(QWebPage::Forward));
	connect(pcmdHome,SIGNAL(clicked()),this,SLOT(home()));

	ptxtBrowser->setUrl(url);

#endif

	pcmdBack->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	pcmdForward->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	pcmdHome->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

	//Layout setup
	QVBoxLayout* pvbxLayout = new QVBoxLayout;
	QHBoxLayout* phbxLayout = new QHBoxLayout;
	phbxLayout->addWidget(pcmdBack);
	phbxLayout->addWidget(pcmdHome);
	phbxLayout->addWidget(pcmdForward);
	pvbxLayout->addLayout(phbxLayout);
	pvbxLayout->addWidget(ptxtBrowser);
	setLayout(pvbxLayout);
}

helpBrowser::~helpBrowser() {

	// TODO Auto-generated destructor stub
}


void helpBrowser::endLoad(bool ok){
	if(ok){
#ifdef _WEBKIT_
		QWebHistory* pHistory =ptxtBrowser->history();
		pHistory->clear();
#endif
   }

#ifdef _WEBKIT_
	disconnect(ptxtBrowser,SIGNAL(loadFinished (bool)),this,SLOT(endLoad(bool)));
#endif
}

void helpBrowser::home(){

#ifdef _WEBKIT_
	connect(ptxtBrowser,SIGNAL(loadFinished (bool)),this,SLOT(endLoad(bool)));
	ptxtBrowser->setUrl(url);
#endif
}


