/*
 * mdata.h
 *
 *  Класс-описание параметров данных прибора
 *
 *
 *  Created on: 30.10.2012
 *      Author: Usach
 */

#ifndef MDATA_H_
#define MDATA_H_

#include <dgserver_global.h>

#include <QtCore>
#include <QObject>


//******************************************************************
template <class Key, class T> class IniQMap : public QMap<Key,T>
{
public:
    inline IniQMap<Key,T> &operator<< (const QPair<Key,T> &t)
    { this->insert(t.first,t.second); return *this; }
};

template <class T> class IniQVector : public QVector<T>
{
public:
    inline IniQVector<T> &operator<< (const T &t)
    { this->append(t); return *this; }
};

template <class T> class IniQList : public QList<T>
{
public:
    inline IniQList<T> &operator<< (const T &t)
    { this->append(t); return *this; }
};

class IniQStringList : public QStringList
{
public:
    inline IniQStringList &operator<< (const QString &t)
    { this->append(t); return *this; }
};
//*************************************************************************


class DGSERVER_EXPORT mdata;

class  mdata {

public:

	static const int WA_HEDSIZE =20;
	static const int EO_HEDSIZE =44;
	static const int VO_HEDSIZE =44;

	static const char Count_ADC=6;//5;
	static const char Count_DAC=16;
	static const char Count_Rele=16;
	static const char Count_DR_Slot=8;
	static const char Count_Tah=3;
	static const char Count_Vir=2;
	static const char Count_Termo=3; //0;
	static const char Count_GE=8;

    static const QVector<QString>	 snslot;    //список имен слотов
    static const QVector<QString>	 snkan;  	//список типов Каналов
    static const QVector<QString>    stipt;		//список типов токовых выходов
    static const QVector<QString>    stipu;		//список типов  выходов по напряжению
    static const QVector<QString>    snust;  	//список названий уставок
    static const QVector<QString>    avfilter;	//список фильтров канала А
    static const QVector<QString>    avHIfilter;	//список фильтров канала А
    static const QVector<QString>    avLOfilter;	//список фильтров канала А
    static const QVector<QString>    rvfilter;	//список фильтров канала C
    static const QVector<QString>    s1tah;		//список режимов работы первого тахометра
    static const QVector<QString>    sitah;		//список источников сигнала тахометра (без каналов М)
    static const QVector<QString>    sge;		//список груповых событий

    static const  QMap<QString,QStringList>   mvparam;	//словарь допустимых параметров для типа канала
    static const  QMap<QString,QStringList>   daparam;	//словарь допустимых параметров для токовых выходов
    static const  QMap<QString,QString>       moparam;	//словарь параметров - название,описание


	static QString tr(const char *sourceText, const char *comment = 0, int n = -1){
	 	 return QObject::tr(sourceText,comment, n);
	}
	static QVector<QString>  getGE(){
		QVector<QString> rez(Count_GE);
		QString txt;
		for(int i=0;i<Count_GE;i++)   rez[i]="GE"+txt.setNum(i+1);
		return rez;
	}
};


#endif /* MDATA_H_ */
