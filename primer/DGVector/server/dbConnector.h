/*
 * dbConnector.h
 *
 *  Created on: 20 окт. 2014 г.
 *      Author: Usach
 */

#ifndef DBCONNECTOR_H_
#define DBCONNECTOR_H_


#include <QtCore>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDomNode>


class dbConnector : public QObject{


public:
	dbConnector(const QString&, QObject *parent=0);
	virtual ~dbConnector();

	virtual bool dbConnect(QDomElement,QString&);
	void dbClose();
	QDomElement getConPar(){ return set;};
	int getStatus(){return stat;};
	QString getNameCon(){return nameCon;};

protected:

	int stat;

	QDomElement   set;//
	QString nameCon;

};

#endif /* DBCONNECTOR_H_ */
