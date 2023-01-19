/*
 * info.h
 *
 *  Created on: 17.10.2012
 *      Author: Usach
 */

#ifndef INFO_H_
#define INFO_H_

#include "former.h"
#include "report.h"
#include "../parser/parser.h"
#include "../mdata/mdata.h"
#include <QObject>

class info : public QObject, mdata{

	 Q_OBJECT

public:
	info(QString tmpl);
	virtual ~info();
	void ViewGFG(QString cfg);
	report *rep;  //окно вывода


signals:
    void Close(int);


private:
	former *form; //обработчик шаблонов
	parser	arrs; //парсер конфига
	gparser arrg;
	QString ftxt; //содержимое шаблона
	QString ptxt;//результат

	bool ParseForm(QString*);
};

#endif /* INFO_H_ */
