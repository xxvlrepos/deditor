/*
 * mMap.h
 *
 * Хранилище данных - зеркало модбас-регистров с потокобез. доступом
 *
 *  Created on: 24 июня 2014 г.
 *      Author: Usach
 */

#ifndef MMAP_H_
#define MMAP_H_

#include <mdata.h>
#include <execut.h>


class infoADC{
public:
	QVector<unsigned char> tah;
	QVector<unsigned char> adc;
	QVector<unsigned char> vir;
	QVector<unsigned char> term;
	QVector<QVector<unsigned char> > ath;
	infoADC(){
		tah=QVector<unsigned char>(mdata::Count_Tah,0);
		adc=QVector<unsigned char>(mdata::Count_ADC,0);
		vir=QVector<unsigned char>(mdata::Count_Vir,0);
		term=QVector<unsigned char>(mdata::Count_Termo,0);
		ath=QVector<QVector<unsigned char> >(mdata::Count_ADC,QVector<unsigned char>(mdata::Count_Tah,0));
	}
};


/*
 * Потокобезопастный класс-таблица модбас
 */
class  MArray : public QByteArray{
public:

	static const int maxFAdr=0x270F;//максимальный адрес файла (0...0x270F)
	static const int maxBAdr=0x4FFF;//максимальный адрес буфера
	static const int Shift=0x1000;  //Начальный адрес
	static const int dSize=(maxBAdr-Shift)*2; // Физич размер буфера
	//C 0x5000 идут адреса закладки 125 регистров в блоке.


	enum eStat{eStop=0,eInfo,eData}; //тек. состояние. Не оновляется, получаем Информацию, пол. Данные
	enum kType{kNONE=0,kA,kC,kD,kI,kT};//только для старых приборов одноканальных

	QString id;
	QString ip,modbus,port,speed,comaddr,ver;
	QString tipCon;
	QString alias;
	unsigned char imodbus;
	int  ispeed;
	QDate vdat;
	eStat status;
	bool dstatus;
	bool Debug;
	bool Tdebug;
	infoADC inf;
	execut* exe; //указатель на класс-читатель прибора;
	qint64 endtime;//время обновления - для снифера
	kType type;  //только для старых приборов одноканальных
	QString emulation;

	MArray() : QByteArray(dSize,0){
			status=eStop;
			dstatus=false;
			Debug=false;
			Tdebug=false;
			exe=NULL;
			endtime=0;
			type=kNONE;
	}
	void setModbus(QString m){bool ok; modbus=m;	imodbus=m.toUInt(&ok,10);}
	void setSpeed(QString b){bool ok; speed=b;	ispeed=b.toUInt(&ok,10);}
	QByteArray midM( int pos, int len = -1 );
	void replaceM(int pos,const char* a,unsigned char alen);
	void replaceM(int pos,const QByteArray & a);
	void  setMData(int adr,short d);
	short getMData(int ma);
	QVector<short> getMData(int ma,int len);
	static void setBad(QByteArray& res,unsigned char bad);
	void fun04(QByteArray& com,QByteArray& res);
	void updateData(){endtime=QDateTime::currentMSecsSinceEpoch();}


protected:

	QMutex mutex;
	union usort{
		short sd;
		unsigned char sc[2];
	};

};


//*****************************************
class  mMap;
//***************************************
class  mMap : public QHash<uchar,MArray*> {
public:
	//общие параметры ретрансляции пакетов
//	static int Repeat;    //число повторов при потере
	static int TimeOut;   //ждем ответ
	static mMap* mdbMap;
	mMap(){countShed=0; mdbMap=this;}
	virtual ~mMap(){ clear(); }


	QMap<unsigned char,int>  getStatus();
	QMap<unsigned char,bool> getDStatus();

	//это служебные функции!!!
	void clear();
	void setParam(int TimeO=100){TimeOut=TimeO;}
	void updateData(const QByteArray& sr,const QByteArray& re);
	int countShed;   //сюда запоминаем число заданий по выкачке осцилограмм

	//Vector протокол
	void command_x01(unsigned char,const char*);
	void command_x02(unsigned char,const char*);
	void command_x03(unsigned char,const char*);
	void command_x04(unsigned char,const char*);
	void command_x07(unsigned char,const char*);
	void command_x09(unsigned char,const char*);//уставки
	void command_x11(unsigned char,const char*);
	void command_x14(unsigned char,const char*);

protected:

	static short getMArrData(const QByteArray&,int); //хватаем short из указанной позиции битового массива
	//обьединение для вычисления crc
	union usort{
		short sd;
		char sc[2];
	};

	union ucrc{
		unsigned short	  shor;
		char			  cha[2];
		bool			  bo[16];
		signed   short	  ssh;
	};
};

#endif /* MMAP_H_ */
