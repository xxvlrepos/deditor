/*
 * former.h
 *
 *  Created on: 08.06.2010
 *      Author: Usach
 *
 *    Класс для обработки шаблонов документов.
 *    Поддерживает переменнные типа {} и боки вида
 *    <!-- BEGIN Name --> <!-- END Name -->
 */

#ifndef FORMER_H_
#define FORMER_H_

#include <QtCore>


class former {
public:
	former(QString* str); //ссылка на возвращаемую строку
	// str - получаемая строка
	virtual ~former();

private:
	QString*				pstr; //Выходная строка
	QMap<QString,QString>  	keyvar; //массив переменных

public:
	void set_txt(QString,QString*); //сброс старого и установка нового текста
	//1-имя текста, 2-значение
	void set_block(QString, QString, QString); //вытаскиваем именованный блок из переменной с заменой на соответствующую
	//1-где ищем, 2-что ищем, 3-на что меняем
	void set_var(QString, QString, bool);   //задаем значения переменных
	//1-имя переменной, 2-значение, 3-заменяем (false) или добавляем - true
	void parse(QString, QString, bool);// в строку по имени 1 парсим строку по имени 2 с добавлением или заменой
	//1-имя результата,2-имя строки для парсинга,3-замена в строке результата (false) иначе true - добавление
	void get_undefined(QString, QString); //ищем переменные в строке и кладем в массив переменных
	//1-имя строки-где ищем, 2-присваиваем значение (по умолчанию - пусто)
	void get_vars(QString); //получаем строку по имени-результат в во входной строке класса str
	//1 - имя строки

};

#endif /* FORMER_H_ */
