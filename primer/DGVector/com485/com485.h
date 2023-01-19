/*
 * com485.h
 *
 *  Created on: 01 июля 2015 г.
 *      Author: Usach
 */

#ifndef COM485_COM485_H_
#define COM485_COM485_H_

#include <QtSerialPort/QSerialPort>
#include <QTimer>
//#include <QtSerialPort/QSerialPortInfo>
#include <dgserver_global.h>



enum Prot485 {Modbus=0,Vector=1};

class DGSERVER_EXPORT com485: public QSerialPort{


//class com485: public QSerialPort{

	Q_OBJECT
	//обьединение для вычисления crc
	 union ucrc{
		unsigned short	  ushor;
		char			  cha[2];
		bool			  bo[16];
		signed   short	  shor;
	 };



public:

#ifndef _WIN_
	static const int dWait = 15;
#else
	static const int dWait = 50;
#endif

	static uchar  debugN;

    explicit com485(Prot485 prot=Modbus,QObject *parent = 0):QSerialPort(parent){init(prot);}
    explicit com485(const QString &name,Prot485 prot=Modbus, QObject *parent = 0):QSerialPort(name,parent){init(prot);}
    explicit com485(const QSerialPortInfo &info,Prot485 prot=Modbus, QObject *parent = 0):QSerialPort(info,parent){init(prot);}
    void setDWait(uint d){wait=d;}
	virtual ~com485(){}
	QString ByteArray_to_String(const QByteArray&);
	Prot485 getProt(){return Prot;}
	void closeVector();

signals:
   void dataRead(const QByteArray&);
   void errorStr(QString);

public  slots:
    void dataWrite(const QByteArray&);



private:
	QByteArray	   bsend;     //буфер запроса
    QByteArray     bresp;     //буфер ответ
    bool           lis;       //признак "слушаем"
    int wait;
    unsigned short crc16; //сохраняем CRC;
    Prot485 Prot;
    ushort startCRC[2];

    void init(Prot485 prot=Modbus);
    QTimer timer;
    uchar M,F,PF;
    int lenLis;
    bool VektRez;

    qint64  bytesWritten;

    void UpdateCRC16Modbus(unsigned char, unsigned short*);   //Функция обновления CRC16 входящим байтом. для modbus
    unsigned short CRC16ModbusArr(const QByteArray&,int len=0);//вычисление CRC16 для modbus
    unsigned short CRC16Modbus(const char* d,int len);


    void UpdateCRC16Vector(unsigned char, unsigned short*);
    unsigned short CRC16VectorArr(const QByteArray&,int len=0);
    unsigned short CRC16Vector(const char* d,int len);



    void AddCRC16(QByteArray&);//добавление контрольной суммы
    bool TestCRC16(const QByteArray&);           //проверка CRC16
    bool TestAddCRC16Modbus(const char* d,int len,unsigned short* crc);

    int getLenS(uchar F,uchar PF,char*); //размер по передаче
    int getLenR(uchar F,uchar PF,char*); //размер по приему
    int getLenV(const QByteArray&);      //размер по передаче Vector

    void resiveData(QByteArray& arr,int len,bool ok);

private slots:
	void readCOM();
	void handleBytesWritten(qint64);
	void handleTimeout();
	void handleError(QSerialPort::SerialPortError);
};

#endif /* COM485_COM485_H_ */

