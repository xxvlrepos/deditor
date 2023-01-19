/*
 * comport.h
 *
 *  Created on: 01.12.2009
 *      Author: Usach
 */

#ifndef IPPORT_H_
#define IPPORT_H_

#include <QObject>
#include <QTimer>
#include <QTextCodec>
#include <QByteArray>
#include <QtNetwork>


class ipport:  public QObject
{
	Q_OBJECT
public:
  QTcpSocket					socket;		  //сокет
  QString						adr;		  //IP- адрес
  unsigned short int			port;	 	  //адрес порта
  bool							debug;       //режим отладки
  int							Desyn;         //Десихронизация передачи

   #ifdef _WIN_
  	  static const int delay = 400;                //период ожидания
   #else
  	  static const int delay = 200;	             //период ожидания
   #endif

  static const int  rbuf = 3072;			     //max размер буфера считывания за раз
  static const char end_ch='\r';	             //завершающий символ строки конца ввода
  //для работы с буферизацией
  static const int  sdelay =   1;				 //ожидание бита для посылки


private:
  QTimer timer;              //таймер чтения
  QTimer stimer;             //таймер посылки одиночного бита
  QTextCodec *codec;         //перекодировщик
  QByteArray b_read;   	     //буфер чтения для строковых данных (замена нулей на пробелы)
  QByteArray b_raw_read;   	 //буфер чтения сырых данных
  QTime      taut;           //таймер таимаута ожидания данных - только при отладке!
  qint64     putCou,wriCou; //глобальный счетчики принятых для посылки и отосланный байт



public:
	ipport();							   	//конструктор
	virtual ~ipport();				   		//диструктор
	bool transmitMsg(const QString&);   	//передать команду целиком c добавлением ENTER
	bool transmitMsg(const QByteArray&);
	QString  					sendMsg;   	//Строка сообщения (без зав. ENTER) посылаемая по слоту sendPort()
	void setDebug(bool);


signals:
	void replyMsg(const QString&);     //ответ из порта строкой
	void replyArr(const QByteArray&);  //ответ из порта массивом
	void replyFArr(const QByteArray&); //ответ из порта массивом без задержки
	void replyCh(const char&);        //из порта посимвольно
	void Open(bool);					//порт открыт
	void desync(const int);          //сигнар рассинхронизации в байтах

	void debugSig(const QString&);    //строка отладки - текстовое собвтие
	void debugSend(const int);       //строка отладки
	void debugRead(const int);       //строка отладки
	void debugTime(const int);       //показ задержек



public slots:
	bool openPort();     				//сигнал для открытия
	bool openPort(bool);				//открыть/закрыть
	void closePort();    				//сигнал для закрытия
	void ipConnect();					//перехватывающие слоты
	void ipDisconnect();				//перехватывающие слоты
	void sendRead();	                //принудительное "очистка" буфера
	void setIpPort(const QString&);	//смена IP адреса порта - нужен реконект
	void putPort(const char&);		//посылает в порт один байт
	void putPort(const QString&);     //посылает в порт строку
	void putPort(const QByteArray&);  //посылает в порт битовый массив
	void putMsg(const QString&); 		//передать команду целиком c добавлением ENTER
	void sendPort();				    //посылает в порт строку sendMsg;
	void receiveMsg();                //вычитывает порт по сигналу прихода данных
	void Writen(qint64);				//когда данные введены/ушли...
	void sendBuf();					//посылает в порт накопленный буфер по таймауту или заполнению

};

#endif /* IPPORT_H_ */
