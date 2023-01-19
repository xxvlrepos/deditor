/*
 * DBTable.cpp
 *
 *  Created on: 31 янв. 2018 г.
 *      Author: Usach
 */

#include "DBTable.h"


DBTable::DBTable(QString N) {
	Name=N;
	Create="CREATE TABLE";
}

DBTable::~DBTable() {
	// TODO Auto-generated destructor stub
}

QString DBTable::greate(){
    QString rez=Create+" "+Name+" (";
    for(int i=0;i<Column.size();i++){
    	colum &c=Column[i];
    	rez.append(" "+K+c.Name+K+" "+c.Prop+",");
    }
    QString SKey=getKey();
    if(Key!="" && SKey!="") rez.append(" "+Key+" ("+SKey+")");
    else        rez.truncate(rez.size()-1); //убрали запятую
    rez.append(")");
    if(Prop!="")  rez.append(" "+Prop);
    rez.append(";");
	return rez;
}

QString DBTable::drop(){
		return "DROP TABLE "+Name+" ;";
}

QString DBTable::insert(){
	 QString str="INSERT INTO "+Name+" VALUES ";
	 return str+getVList()+";";
}

QString DBTable::replace(){
	 QString str="REPLACE INTO "+Name+" VALUES ";
	 return str+getVList()+";";
}

QString DBTable::update(){
		return "";
}

QString DBTable::getVList(){
	 QString str="(";
	 for(int i=0;i<Column.size();i++){
		 if(Column[i].V!="") str.append(Column[i].V);
		 else                str.append("DEFAULT");
		 if(i<Column.size()-1) str.append(",");
		 else                  str.append(")");
	 }
	 return str;
}

QString DBTable::getColumEQ(int i){
	if(i>=Column.size()) return "";
	if(Column[i].V=="")  return "";
	return Column[i].Name+"="+Column[i].V;
}

void DBTable::setValue(int i,QString v){
	if(i>=Column.size()) return;
	Column[i].V=v;
}

QString DBTable::getKey(){
	QString rez="";
	QList<colum> sp;
	bool s=true;
	for(int i=0;i<Column.size();i++){
		if(Column[i].Key>0) sp<<Column.at(i);
	}
	if(sp.size()==0) return "";
	//сортировка
	while(s){
		s=false;
		for(int i=0;i<sp.size()-1;i++){
			if(sp[i].Key>sp[i+1].Key){
				sp.swap(i,i+1);
				s=true;
			}
		}
	}

	for(int i=0;i<sp.size();i++){
		rez.append(K+sp[i].Name+K);
		if(i<sp.size()-1) rez.append(", ");
	}
	return rez;
}
