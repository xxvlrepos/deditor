/*
 * Sheduler.h
 *
 * Дергаем по расписанию выкачку осцилограмм
 *
 *  Created on: 09 окт. 2014 г.
 *      Author: Usach
 */

#ifndef SHEDULER_H_
#define SHEDULER_H_

#include <QtCore>
#include <QObject>


class JobT{

public:
	JobT();

	bool addShed(QString m, QString h);
	bool test(QDateTime);

protected:
	QList<unsigned char> min;
	QList<unsigned char> hour;

	QDateTime testDT;
	QDateTime newDT(QDateTime);

	static QList<unsigned char> parser(QString d,int max,bool *rez);

};

//***************************************************************

class Sheduler : public QObject{

	Q_OBJECT

public:

	static const int timeInt=1000;

	Sheduler(QObject *parent = 0);
	virtual ~Sheduler();

	bool append(QString m, QString h);
	void clear();

	//bool iniSheduler(const QString& Name,const QString& xsdName);

public slots:
	void start();



protected:

	QList<JobT> shedList;
	QTimer timer;
	QDateTime curDTime;


	int lifeTime;

signals:
	void startJob(int);

protected slots:
	void test();

};

#endif /* SHEDULER_H_ */
