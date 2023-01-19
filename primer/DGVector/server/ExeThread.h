/*
 * ExeThread.h
 *
 * Поток опроса для одной шарманки
 *
 *  Created on: 02 окт. 2014 г.
 *      Author: Usach
 */

#ifndef EXETHREAD_H_
#define EXETHREAD_H_

#include <QtCore>
#include <QObject>
#include <execut.h>
#include <QThread>
#include "mMap.h"
//*****************************************************************************
/*

 */

class ExeThread: public QThread
{
     Q_OBJECT

public:

    execut* exe;

    ExeThread(tape*tp, QObject *parent=0);
    ~ExeThread();
    void run();

    virtual void exeRez(command* cmd);

signals:
   void fileLoaded(QString);

public slots:
   void halt();

protected slots:
    void exit();
    void headRez(command*);

protected:
	tape* ttp;

	signals:
	void stopEXE();
};


#endif /* EXETHREAD_H_ */
