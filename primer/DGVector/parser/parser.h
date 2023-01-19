/*
 * parser.h
 *
 * Парсинг и разбор файлов конфигурации прибора
 *
 *  Created on: 10.10.2012
 *      Author: Usach
 */

#ifndef PARSER_H_
#define PARSER_H_
#include <QWidget>
#include <QtCore>

struct pstr{
       	int 			n;//номер строки
       	QStringList     p;//список параметров
};
struct gstr{
	    	int				n;//номер строки в файле
	    	QString			comm;//коментарий
	    	QString			exp;//Выражение
	    	QString			err;//Текст ошибки
	    	QStringList		age;//Выражение, разобранное по операндам
	    };

typedef QMap<QString,gstr>  GeParse; //Массив разобранных групповых событий
typedef QMap<QString, pstr> TParse;


class parser: public TParse {
public:
	parser();
	parser(QString);
	virtual ~parser();
	bool ReadCFG(QString);

};

class gparser: public GeParse {
public:
	gparser();
	gparser(QString);
	virtual ~gparser();
	bool ReadGE(QString);
	QStringList		sge;		//список груповых событий
private:


};

#endif /* PARSER_H_ */
