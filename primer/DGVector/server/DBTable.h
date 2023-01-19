/*
 * DBTable.h
 *
 *  Created on: 31 янв. 2018 г.
 *      Author: Usach
 */

#ifndef DBTABLE_H_
#define DBTABLE_H_

#include <QString>
#include <QList>

class DBTriger{
public:
	QString Name,SrcOb,RezOb;
	DBTriger(QString N,QString srcOb,QString rezOb){
		Name=N;	SrcOb=srcOb; RezOb=rezOb;
	};
	virtual ~DBTriger(){};
	virtual QString greate(){return "";};
	virtual QString greateF(){return "";};

};

class DBProc{
public:
	QString Name;
	DBProc(QString N){Name=N;};
	virtual ~DBProc(){};
	virtual QString greate(){return "";};
	virtual QString drop(){return "";};
	virtual QString call(QString){return "";};
};


class DBTable {
public:
	struct colum{
		colum(QString N,QString P,unsigned char K=0){ Name=N; Prop=P; Key=K;}
		QString Name;
		QString Prop;
		uchar   Key;
		QString V;
	};

	DBTable(QString N);
	virtual ~DBTable();

	QString Name;
	void addColum(QString N,QString P="",unsigned char K=0){Column<<colum(N,P,K);}
	void setValue(int,QString);
	QString getVList();
	virtual QString greate();
	virtual QString insert();
	virtual QString replace();
	virtual QString drop();
	virtual QString update();

	QString getColumEQ(int);


protected:
	    QString Key;
		QString Prop;
		QString Create;
		QList<colum> Column;
		QString getKey();
		QString K;  //обратная кавычка
};



class DBTableM : public DBTable{
public:
	 DBTableM(QString N): DBTable(N){
		K="`";
		Key="PRIMARY KEY";
	 }
};

class DBTableP : public DBTable{
public:
	 DBTableP(QString N): DBTable(N){
		K="";
		Key="CONSTRAINT "+N+"_pkey PRIMARY KEY";
	 }
};


#endif /* DBTABLE_H_ */
