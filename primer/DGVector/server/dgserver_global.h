#ifndef DGSERVER_GLOBAL_H
#define DGSERVER_GLOBAL_H

#include <QtCore/qglobal.h>

//версия для непрервыной диагностики
// скачивание непрерывно циклически, а не по расписанию

//#define _UNSHED_DIAG_


#ifdef DGSEVER_LIB
  #define DGSERVER_EXPORT  Q_DECL_EXPORT
#else
  #define DGSERVER_EXPORT  Q_DECL_IMPORT
#endif


#endif // DGSERVER_GLOBAL_H
