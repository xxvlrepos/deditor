/*
 * parser.cpp
 *
 *  Created on: 10.10.2012
 *      Author: Usach
 */

#include "parser.h"
#include "../mdata/mdata.h"

parser::parser(){}

parser::parser(QString Name){
	ReadCFG(Name);
}

parser::~parser() {
	// TODO Auto-generated destructor stub
	clear();
}

/**
 * Читаем файл конфигурации
 * Параметры: имя файла,
 * Результаты: строковый массив файла, массив ключей параметров
 */
bool parser::ReadCFG(QString Name){

	QFile 		file; //файловые переменные
	QString 	line,txt; //строка для разбора


	QTextCodec*	codec= QTextCodec::codecForName("IBM866"); //установка перекодировщика

	//читаем в массив
	file.setFileName(Name);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;

	//очищаем словарь разобранной конфигурации
	clear();

	int nst=0;
	while(!file.atEnd()){

		line=codec->toUnicode(file.readLine());
		if(nst==0){	if(line.left(3)!="ID:") return false;}
		nst++;
		if(line.contains("=")){
			txt=line.section("=",0,0).trimmed().toUpper();
			pstr pn;
			pn.p=line.section("=",1,-1).trimmed().split(",");
			pn.n=nst;
			insert(txt,pn);
		}else{
			if(line.contains(":"))	{
				txt=line.section(":",0,0).trimmed().toUpper();
				pstr pn;
				pn.p=line.section(":",1,-1).trimmed().split(",");
				pn.n=nst;
				insert(txt,pn);
			}
		}
	}
	file.close();
	return true;
}


gparser::gparser(){
	QString txt;
	for(int i=0;i<mdata::Count_GE;i++)   sge<<"GE"+txt.setNum(i+1);

}

gparser::gparser(QString Name){
	gparser();
	ReadGE(Name);
}

gparser::~gparser() {
	// TODO Auto-generated destructor stub
	clear();
}

/**
 * Разбор файла групповых событий в глобальный массив arrg
 */
bool gparser::ReadGE(QString gename){

	QFile 		file; //файловые переменные
	QString 	line; //строка для разбора
	QString 	txt,txt1,txt2,key;
	QTextCodec*	codec;
	codec = QTextCodec::codecForName("IBM866");
	QMap<QString, gstr*>  mg;


	clear();
	//грузим групповые
	if(gename!=""){
	   //разбираем групповые события в массив
	   file.setFileName(gename);
	   if(file.open(QIODevice::ReadOnly | QIODevice::Text)){
			int str=0;
			while(!file.atEnd()){
				str++;
				line=codec->toUnicode(file.readLine().trimmed());
				//При чтении файла не разбираем сперва по полям - так как могут быть переносы
				if(line.contains("=")){
					txt=line.section("=",0,0).trimmed().toUpper();
					mg.insert(txt,new gstr);
					mg[txt]->exp=line.section("=",1,1).trimmed();
					mg[txt]->n=str;

				}else{
					mg[txt]->exp+=line.trimmed();
				}
			}
			file.close();
		}else return false;

		 //а теперь разбираем по параметрам
	   	for(int i=0;i<sge.count();i++){
	   		key=sge.at(i);
	   		if(mg.contains(key)){
	   			mg[key]->comm=mg[key]->exp.section(",",0,0).section("\"",1,1);
	   			mg[key]->exp=mg[key]->exp.section(",",1,1).section("\"",1,1);
	   			//разбираем
	   			txt=mg[key]->exp;
	   			txt2="";
	   			for(int s=0; s<txt.size();s++){
	   				txt1=txt.at(s);
	   				if(txt1=="(" || txt1==")" || txt1=="&" || txt1=="|" || txt1=="!"){
	   					if(txt2!="") {
	   						mg[key]->age<<txt2.trimmed();
	   						txt2="";
	   					}
	   					mg[key]->age<<txt1;
	   				}else{
	   					txt2+=txt1;
	   				}
	   			}
	   			if(txt2!="") mg[key]->age<<txt2;
				insert(key,*(mg[key]));
	   		}
	   	}
	}else return false;
	return true;
}
