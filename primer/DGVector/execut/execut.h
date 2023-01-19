/*
 * execut.h
 *	Командный процессор для выполнения ленты команд
 *
 *  Created on: 13.07.2012
 *      Author: Usach
 *
*/

#ifndef EXECUT_H
#define EXECUT_H

#include "dgserver_global.h"

#include <QtCore>
#include <QtNetwork>
#include "com485.h"
#include <QElapsedTimer>
#include "tape.h"                   //общие структуры и классы


 //обьединение для данных (универсальное) на 8 байт
union lon{
	unsigned long long int din;
	unsigned long int	   in[2];
	char			  	   cha[8];
	bool				   bo[64];
};

/*
#ifdef _WGCC_
  class  execut;
#else
  class DGSERVER_EXPORT execut;
#endif
*/
 
class DGSERVER_EXPORT execut : public QObject
{
    Q_OBJECT

public:
    execut(uchar deb=0,QObject *parent = 0);
    ~execut();

     enum transport{NONE=0,COM,TCP,UDP,EXT};

    static  uint comDelay; //принудительная задержка отправки для com-порта
    static  uint comMWait; //максимум ожидания в драйвере (разрвы посылки)

    command* append(commandType t,QString l="") {return Tape->append(t,l);}
    command* last(){return Tape->last();}

    execut*         extExe;//внешняя шарманка

    void setTape(tape* T=0); //выбор листа выполнения (внешний или внутренний)
    tape* getTape(){return Tape;}
    bool getStatus(){return run;} //получить статус
    int  getNStat(){return gcou;}
    int  getTransport(){return transp;}//транспорт
    bool Start();    //запуск шарманки
    void Stop(bool del=false);     //останов
    QString getAddress();//получить адрес  соединения (com или IP)
    QString getPort();//получить порт соединения (IP)
    void extReceive(const QByteArray& res){ if(transp==EXT) emit resExt(res);}
    void enDebug(uchar d){ debug=(d>0); dadr=d;}
    uchar getDebug(){return dadr;}
    void enTdebug(bool dt){ tdebug=dt;}


    //вспомогательные функции
    static QString ByteArray_to_String(const QByteArray&);  //битовый массив в строку
    static QByteArray convert(unsigned long long int,int); 	//преобразует число в битовый массив
    static QByteArray convert(const QString& str);//преобразует строку hex (без. преф) в битовый массив
    static long long int aconvert(QByteArray,int,int);  //обратное-битовый массив в число

public slots:
    void exeCommandE(dIF*);//исполнитель внешних команд внешней шарманки - неблокирующий
    void start(){Start();}     //запуск
    void stop(){ qDebug()<<"sig stop"; Stop();}       //принудительный останов;
  //  void extReceive(dIF*,const QByteArray& res){ extReceive(res); }

signals:
	void statScharman(bool);		 //сообщает состояние шарманки
	void finish(int);                //передает окончание и метку
	void statScharman(int);		     //сообщает этап
	void statDownload(int);          //процент выкачки файла
	void errorDownload(int);         //ошибка выкачки файла
	void timeAct(int);               //время выполнения заданной цепочки комманд
	void timeCycle(uchar,int,int);    //время выполнения цикла осредненное
	void Onlain(bool);               //наличие установленной связи/открытого порта/слота
    void rezult(command*);           //результат
	void messageError(QString);      //сообщения о ошибках

protected:
	bool	debug;//запуск с включенным "сырым/отладочным" функционалом
	bool    mdebug; //отладка по поводу уничтожения указателей
	uchar   dadr;
	bool    tdebug;//выводить время цикла - если включено
	int     couTD;//делитель для осреднения времени
	int     couT;//счетчик осреднения
	int     sumT;//суммато
	int     endTD;//сколько раз показвыать

    tape			TapeI; //последовательность команд как словарь (встроенный)
    tape*			Tape;  //последовательность команд как словарь (внешний)
    dIF*            extCOM;//внешняя команда

    int		        curWait;//тек время ожидания

    void timerEvent(QTimerEvent *event);
    void clear(){Tape->clear();}

    void SScharmanka(command*);                     //формирование запроса для текущей команды
    void RScharmanka(command*, const QByteArray& rep);//обработка ответа и принятие решения
    void ttaut(command*);   //обработка таймауто ответа


private slots:                                      //ответ не пришел - обработка таймаута
	void Receive(const QByteArray&);                 //получает ответ и вызвать обработку
    void readIPSlot();//вычитка слота
    void putIPSlot(const QByteArray&);
    void readUDPSlot();//вычитка слота
    void putUDPSlot(const QByteArray&);
    void putEXT(const QByteArray&);
    void runTime(bool);
    void error(QAbstractSocket::SocketError);
  //  void stop(int);//сигнал от главной шарманки о останове



signals:
     void putCh(const QByteArray&);   //отправляем  битовую последовательность кому надо (в com порт)
     void pendTime(bool);
     void resExt(const QByteArray&);

private:
     //поток команд
     void exeq();
    //вызов следующей команды по окончании разбора
      void Next(int);
      //открытия соединений
      void udpOpen(bool);
      void ipOpen(bool);
      void comOpen(bool);
      void extOpen(bool);
      bool Send(); //обрамляет посылку суфиксом и префиксом и посылает в нужный тунель
      void disconnect_io(); //рвем все
      void connect_io(dCONNECT* dco);
      void extReceive(command*,const QByteArray&);


      //обьединение для вычисления crc
         union ucrc{
         	unsigned short	  ushor;
         	char			  cha[2];
         	bool			  bo[16];
         	signed   short	  shor;
         };

    //для работы с портом и терминалом
    com485* 				    com;		  //com порт 485
    QTcpSocket*					socket;		  //сокет
    QUdpSocket*					usocket;	  //udp сокет


    QHostAddress                Addr;        //сюда сохраняем параметры tcp/udp соединения
    quint16		                Port;

    QHostAddress                udpAddrSen;    //- кому шлем
    QHostAddress                udpAddrRes;    //с какого адреса слушаем - в общем случае не обязано совпадать с источником (мультикаст рассылка)
    QHostAddress				udpAddrRF;     //откуда пришло

    QHostAddress                udpSAddr;   //источник
    quint16		                udpSPort;



    QString                     comPort;

    //таймер
    int                     taut;          //таймер таимаута ожидания данных
    int		                tautS;         //запуск след. запроса
    int                     tautM;         //таймер для мультикаста
    int                     pendT;
    QElapsedTimer*          time;          //сечем время обмена
    int						after_delay;   //для сохранения задержки пред. команды

    //текущие буфера приемов запросов и ответов
    QByteArray				        gsend;     //массив запроса (без суфиксов и префиксов)
    QByteArray				        rsend;     //массив запроса (с суфиксами и префиксами)
    QByteArray                      resp;     //ответ сырой
    QByteArray                      gresp;    //ответ
    QByteArray                      emptyArr;
    //параметры приема

    QHostAddress sender;
    quint16 senderPort;

    ucrc                       idtran;
    int                        tcpERRCoun;
    int                        RepeatCoun;

    bool						run;     //флаг работы

    int							gcou;     //Глобальный счетчик посылаемых команд. При нуле-останов, при -1 бесконечный цикл
    command*                    curCMD;  //указатель на текущую команду
    QQueue<command*>            queueCMD;//очередь команд
    QMutex                      mutex;
    int							act_time;//сечем время исполнения между команд
    transport					transp;   //транспорт предачи данны

    QString Char_to_FString16(const char ch);
    QString Char_to_FString10(const char ch);
    static  QByteArray convert(lon,int); 	//преобразует число в битовый массив

};



#endif // EXECUT_H
