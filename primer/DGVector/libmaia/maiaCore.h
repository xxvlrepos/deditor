/*
 * maiaCore.h
 *
 *  Created on: 13 янв. 2015 г.
 *      Author: Usach
 */

#ifndef MAIACORE_H_
#define MAIACORE_H_

#include <QtGlobal>
#if QT_VERSION >= 0x050000
  #include "http_headers.h"
#else
  #include  <QHttpHeader>
#endif



namespace maiaCore {

static const int timeLifeMax=15000; //время жизни соединения после пр. активности
int getKepTime(QHttpHeader*);
int getContLeng(QHttpHeader*);

};

#endif /* MAIACORE_H_ */
