/*
 * former.cpp
 *
 *  Created on: 08.06.2010
 *      Author: Usach
 */

#include "former.h"

former::former(QString* str) {
	//очищаем выходную строку
	pstr=str;
	pstr->clear();
}

former::~former() {

}

/* Cброс старого и установка нового текста
 * 1-имя текста, 2-содержание
 */
void former::set_txt(QString name,QString* fstr){

	keyvar.clear();
	keyvar[name].append(fstr);

}

/** Загружает именованный блок из файла и запоминает как переменную $handle
* 1-где ищем, 2-что ищем, 3-на что меняем
*/
void former::set_block(QString name,QString handle, QString varname="")
{
	//проверяем наличие
	if(!keyvar.contains(name)) return;
	//готовим выражение
	QString qhandle = QRegExp::escape(handle);
	QRegExp reg("\\s*<!--\\s+BEGIN\\s+"+qhandle+"\\s+-->(.*)\\s*<!--\\s+END\\s+"+qhandle+"\\s+-->\\s*");

	//ищем и сохраняем найденное
	if(reg.indexIn(keyvar[name])>-1){
		//qDebug()<<handle;
		QStringList str=reg.capturedTexts();
		keyvar[handle]=str[1];
	}
	//заменяем на переменные
	if(varname.trimmed()=="") keyvar[name].replace(reg,"");
	else keyvar[name].replace(reg,"{"+varname+"}");
}

/**
 * Задаем значения переменных
 * 1-имя переменной, 2-значение, 3-заменяем (п.у.) или добавляем - true
 */
void former::set_var(QString name, QString value, bool append = false){
	if(!append) keyvar[name]=value;
	else keyvar[name].append(value);
}

/** Заменяет переменные в строке по имени handle и помещает в target. Если apped-true - добавляет.
* 1-имя результата,2-имя строки для парсинга,3-замена в строке результата (п.у.) иначе true - добавление
*/
void former::parse(QString target, QString handle, bool append = false){
	//проверка
	if(!keyvar.contains(handle)) return;
	//рабочая строка
	QString rstr=keyvar[handle];
	//выражение
	QRegExp reg("\\{([^}]+)\\}");
	int poz=0; //c какой позиции ищем
	while(reg.indexIn(rstr,poz)>-1){
		QStringList lstr=reg.capturedTexts();
		if(keyvar.contains(lstr[1])) rstr.replace("{"+lstr[1]+"}",keyvar[lstr[1]]);
		else poz=reg.pos(0)+1;
	}
    //результат
	if(!append) keyvar[target].clear();
	keyvar[target].append(rstr);

}

/** Ищем переменные в строке и кладем в массив переменных
 * 	1-имя строки-где ищем, 2-присваиваем значение (по умолчанию - пусто)
 */

void former::get_undefined(QString name,QString value="")
{
	//проверка
	if(!keyvar.contains(name)) return;
	//выражение
	QRegExp reg("\\{([^}]+)\\}");
	int poz=0; //c какой позиции ищем
	while(reg.indexIn(keyvar[name],poz)>-1){
		QStringList str=reg.capturedTexts();
		keyvar[str[1]]=value;
		poz=reg.pos(0)+1;
	}
}

/** Получаем строку по имени-результат в во входной строке класса str
 *  1- имя строки в словаре
 */
void former::get_vars(QString name){
	//проверка
	pstr->clear();
	if(!keyvar.contains(name)) return;
	pstr->append(keyvar[name]);
}
