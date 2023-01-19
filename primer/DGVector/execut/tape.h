/*
 * tape.h
 *
 *  Created on: 13.07.2012
 *      Author: Usach
 *
 *		Данный набор классов описывает словарь комманд для выполнения в классе execut
 *
 *      Замечание.
 *      По результатам разбора ответов на команды могут быть выполнены некоторые функции, но есть ограничение-
 *      данные функции должны быть глобальными или статическими внутри класса. Для работы с нестатическими
 *      параметрами класса используется "финт ушами" - в статической функции вызывается глобальный класс
 *      типа emiter через который генерится сигнал, а сигнал же ловится через connect внутри класса и
 *      передается нестатической функции.
 *
 */

#ifndef TAPE_H_
#define TAPE_H_


#include <QtCore>
#include <qhostaddress.h>
#include <com485.h>
#include "dgserver_global.h"


enum connectType {cEXT=0,cCOM=1,cTCP=2,cUDP=3};
enum commandType {
	CONNECT,         //соединение по заданному интерфейсу (com,tcp,udp)
	DISCONNECT,       //рассоединение с текущим интерфейсом
	IF,       //команда с условием по результату ответа (если таймаут ожидания ответа = 0 ответ не разбирается - для UDP- кладется в словарь)
	FOR,      //команда со счетчиком  или внешним условием завершения
	DOWNLOAD,        //выгрузка файла в папку
	MULTICAST,       //разбор ответов из словаря ответов для UDP, всегда "OK" по завершению разбора
	BROADCAST,       //просто отправляем и после таймаута - на следующ.
	SETTIME,         //установит время
	NOOP              //пустышка
};
enum rezType {NOT=-1,BAD=0,OK=1,CON=2,ERRCON=3}; //результаты выполнения
enum errType {NOERR=0,TIMEERR=1,GWTIME=2,GWBUSI=3,GWNOSL=4};//ошиюки связи - нет, таймаут,GW-неответил слейв, GW-занят,GW-недоступен слейв
enum timeCommand{NONE=0,START=1,STOP=2,STOP_START=3};//команды таймеру, засекающему время процесса
enum typeRetrans{NotRet=0,ExternRet,NoBlockRet};//признак ретрансляции пакета

class execut; //предварительное обьявление

class command {

public:
	QString                 Label;      //Метка (если надо)
	QString					goOK_L;     //Метки переходов для переходов по условию
	QString					goBAD_L;    //Метки переходов для переходов по условию
    unsigned int            delay;      //задержка выполнения этой команды в миллисекундах
	unsigned int            wait;       //ожидание ответа, если истекло - переход по ошибке
    unsigned int            after_delay_OK; //задержка выполнения следующей команды в миллисекундах
    unsigned int            after_delay_BAD;//задержка выполнения следующей команды в миллисекундах

	QString 				stOK;       //сообщения по результату
	QString	                stBAD;

	typeRetrans             Retrans;//признак внешней команды
	//QMutex  				mutex;     //блокировщик
	timeCommand				tCommand;  //указание таймеру засекающему время вып. нескольких команд
	QObject*                parentEXE;  //внешний хозяин - исполнитель
	bool                    ControlRez; //признак - давать сигнал о контроле выполнения команды
	command(){
		Type		=DISCONNECT;
		rez			=NOT;
		err         =NOERR;
		delay		=0;
		wait		=100; //когда 0 -не ждем ответ и не разбираем - широковещание
		after_delay_OK=0;
		after_delay_BAD=0;
		goOK		=-1;
		goBAD		=-1;
		tCommand	=NONE;
		parentEXE=NULL;
		Retrans=NotRet;
		ControlRez=false;

	}

	virtual ~command(){};

	commandType getType() {return Type;}
	rezType getRez(){return rez;}
	void setRez(rezType r){rez=r;}
	errType getERR(){return err;}
	void setERR(errType er){err=er;}
	void setOK(int n){goOK=n;}
	void setBAD(int n){goBAD=n;}
	void setLabel(QString l){Label=l.toUpper();}
	QString getLabel(){return Label;}
	int getOK(){return goOK;}
	int getBAD(){return goBAD;}
	virtual QByteArray getRES(){ return QByteArray();} //виртуальная функция
	virtual bool test(const QByteArray& res){ Q_UNUSED(res); rez=OK; return true;}
	commandType             Type;        //тип команды
	rezType                 rez;        //результат выполнения текущей команды - записывается после выполнения
	errType                 err;
	int						goOK;		//адрес след. команды в случ. пол. ответа/удачи
	int						goBAD;      //адрес в случ отрицательного ответа (не совпадения)/не удачи
};





class concommand : public command{
public:
	//структура маски для контроля ответа (позиция,  и чему должен быть равен)
	class maska{
	public:
		unsigned short start;  //первый байт
		QByteArray     rez;    //ожидаемый ответ
		maska(const QByteArray& r,unsigned short s=0){start=s;rez=r;}
		bool test(const QByteArray& res){
			 if(start+rez.size()>res.size()) return false;
			 for(int i=0; i<rez.size();i++){ if(res[start+i]!=rez[i])  return false;	}
			 return true;
		}
	};


	unsigned char           msize;      //контроль длинны ответа 0-неконтр иначе если меньше, то "плохо"
	QList<maska>			vres;       //маски контролируемых параметров - все должны соответствовать
	QHostAddress			AddrRF;     //откуда ответ

	concommand(){
		msize=0;
		AddrRF=QHostAddress(QHostAddress::Null);
	}
	virtual ~concommand(){}
	void addTest(const QByteArray& r,unsigned short s=0){ vres.append(maska(r,s));}
	virtual bool test(const QByteArray& res){
		if(msize>0) {if(res.size()<msize) {rez=BAD; return false;}}
		for(int i=0; i<vres.size(); i++){
			maska& m=vres[i];
			if(!m.test(res)) {rez=BAD; return false;}
		}
		rez=OK;
		return true;
	};

};


class dNOOP: public command{
public:

	dNOOP(){
		Type=NOOP;
		rez	=OK;
	}
};


class dIF : public concommand{
public:


	dIF(){
		cmd.reserve(256);
		data.reserve(256);
		cmd.resize(2);
		Type=IF;
		cmd[0]=cmd[1]=0;
	}
	virtual ~dIF(){}
	void setAdr(unsigned char a){cmd[0]=a;}
	//!!!
	virtual void setFun(unsigned char f){cmd[1]=f;}//!!!!!
	//!!!
	void setAF(unsigned char a,unsigned char f){setAdr(a);	setFun(f);}
	void setAFCmd(const QByteArray& afc){ cmd=afc;}
	//!!!!!!!!!!
	virtual void setAFCmd(unsigned char a,unsigned char f,const QByteArray& c){setAF(a,f);setCmd(c);}
	virtual void setCmd(const QByteArray& c){cmd.resize(2);cmd.append(c);}
	virtual QByteArray getRES(){return cmd;} //фрмирует посылку в момент запроса!!!
	//!!!!!!!!!!!!!!!
	unsigned char adr(){return cmd[0];}
	unsigned char fun(){return cmd[1];}
	unsigned char pfun(){return cmd.size()>2 ? cmd[3] : 0;}
	QByteArray    getAF() { return cmd.left(2);}
	QByteArray    getAFCmd(){ return cmd;}                   //не модифицирует посылку!!!
	QByteArray    getCmd(){ return cmd.right(cmd.size()-2);} //не модифицирует посылку!!!
	QByteArray    getData() { return data;};
	void  append(const QByteArray& a){cmd.append(a);}
	void  append(const char* ch, int len){cmd.append(ch,len);}
	void  append(char ch){cmd.append(ch);}
	void  replace(int pos, const QByteArray & af ){ cmd.replace(pos,af.size(),af);}
	void  replace(int pos, const unsigned char ch ){ if(pos<cmd.size()) cmd[pos]=ch;}
	QByteArray mid(int pos,int l){ return cmd.mid(pos,(pos+l<=cmd.size() ? l : cmd.size()-pos));}
	//!!!
	virtual bool test(const QByteArray& res){
	//	if(res.size()>0) if(cmd[0]!=res.at(0)) return false; //контроль адреса
		bool r=concommand::test(res);
		data.clear();
		data.append(res);
		return r;
	}
	QByteArray     data; //возврат

protected:
	QByteArray      cmd; //посылаем

};


class dFOR  : public dIF{
public:

	QString stSTART;
	int nsz; //число повторов
	bool continueRez;//при каком результате (OK или BAD) продолжать цикл
	unsigned int            after_delay_CON; //задержка выполнения следующей итерации в миллисекундах

	//обьединение для данных (универсальное) на 8 байт
	union lon{
		unsigned long long int din;
		unsigned long int	   in[2];
		unsigned short   	   sh[4];
		char			  	   cha[8];
		bool				   bo[64];
	};

	void(*fCOU)(void);   //выполнить функцию на этапе цикла


	dFOR(){ //по умолчанию продолжать при хорошем ответе
		Type=FOR;
		cou=-1;
		fCOU=NULL;
		n=0;
		nsz=0;
		continueRez=true;
		after_delay_CON=0;
	}
	virtual ~dFOR(){}
	int getCou(){return cou;}
	int getNSt(){return n;}
	int getStat(){return nsz >0 ? (n*100)/nsz : 100;}
	virtual bool test(const QByteArray& res){
		bool r=concommand::test(res);
		if(n==0) cou=nsz;
		if(continueRez){ if(rez==OK  && cou>0) rez=CON;} //продолжить
		else           { if(rez==BAD && cou>0) rez=CON;} //продолжить
		cou--;
		n++;
		return r;
	}


	/*
	 * преобразует число в битовый массив
	 */
	static QByteArray convert(unsigned long long int ch,int l){
		lon u;
		u.din=(unsigned long long int)ch;
		return convert(u,l);
	}
	/*
	 *  битовый массив в число
	 *  Аргументы массив, первый символ, длинна
	 */
	static long long int aconvert(QByteArray ba,int n,int l){
		lon u;
		if(l>8) l=8;
		u.din=0; //забили нулями
		if(ba.count()>=n+l) {//проверка на выход за пределы
			for(int i=0;i<l;i++){
				u.din=u.din<<8;
				u.din+=(unsigned char) ba.at(n+i);
			}
		}
		return u.din;
	}

protected:
	int     cou;        //внутренний счетчик убывания
	int     n;          //номер оборота

	static QByteArray convert(lon u,int l){
		QByteArray rez;
		if(l>8) l=8;
		for(int i=l;i>0;i--) rez.append(u.cha[i-1]);
		return rez;
	}

};




class dDOWNLOAD : public dFOR{
public:
	QString        dirName;     //папка - куда без "/"
	QString        shortName;   //короткое имя - если не указано - автоматически
	QString        beforeName,afterName; //префикс-суфикс имени
	QString        fileName;    //итоговое полное имя файла
	short          proc,oldpr;  //процент готовности


	dDOWNLOAD(){
		Type=DOWNLOAD;
		oldpr=proc=-1;
	}
    virtual ~dDOWNLOAD(){}
	virtual void setFun(unsigned char f){
		cmd[1]=f;
		switch(f){
			case 0x65://работа с осцилограммами
				cmd.resize(15);
				cmd[2]=0x03;
				for(int i=3; i<cmd.size();i++) cmd[i]=0;
				break;
			case 0x68://выкачка конфига
			case 0x69://выкачка ge
				cmd.resize(4);
				for(int i=2; i<cmd.size();i++) cmd[i]=0;
				break;
			case 0x6A://выкачка lut
				cmd.resize(4);
				for(int i=2; i<cmd.size();i++) cmd[i]=0;
				break;

		}
	}
	void setAF(unsigned char a,unsigned char f){setAdr(a);	setFun(f);}
	void setAFCmd(unsigned char a,unsigned char f,const QByteArray& c){setAF(a,f);setCmd(c);}
	void setCmd(const QByteArray& c){
		if((unsigned char)cmd[1]==0x65) cmd.resize(3);
		else             cmd.resize(2);
		cmd.append(c);}



	virtual bool test(const QByteArray& res){
		bool r=concommand::test(res);
		if(!r) return r;
		int sz;
		switch(fun()){
			case 0x65://работа с осцилограммами
				nsz=aconvert(res,2,2);
				replace(13,convert(n+1,2));
				break;
			case 0x68://выкачка конфига
			case 0x69://выкачка ge
				nsz=aconvert(res,2,2);
				replace(2,convert(n+1,2));
				break;
			case 0x6A://выкачка lut
				nsz=aconvert(res,2,2);
				replace(3,convert(n+1,2));
				break;

		}
		if(n==0){
			if(nsz>0) data.reserve(nsz*248);
			data.clear();
			getName(res);
			cou=nsz;
		}
		sz=aconvert(res,4,2);
		oldpr=proc;
		if(sz>0){
			data.append((res.data()+6),sz);
			cou--;
			n++;
			proc=n*100/nsz;
			if(rez==OK) rez=CON;
		}else{
			proc=100;
			if(data.size()>0) seveFile();
			data.squeeze();
		}
		return r;
	}

protected:

	QString ByteArray_to_String(const QByteArray& arr){
		QString st,fst;
		for(int i=0;i<arr.size();i++){
				//разбор байтов
				st.setNum(arr.at(i),16);
				if(st.size()<2) st="0"+st;
				st=st.right(2);
				fst.append(st);
		}
		return fst;
	}
	void getName(const QByteArray& rep){

		if(fileName.size()!=0) return;

		fileName=dirName+"/"+beforeName;
		if(fun()==0x65){//работа с осцилограммами
			QString tip=ByteArray_to_String(rep.mid(10,1)).toUpper();//уточняем имя
			if(tip.left(1)=="2") tip="M"+tip.right(1);
			if(shortName==""){
				lon time;
				QByteArray a(rep.mid(34,4));
				time.din=0;
				time.cha[0]=rep.at(34);
				time.cha[1]=rep.at(35);
				time.cha[2]=rep.at(36);
				time.cha[3]=rep.at(37);
				shortName=QString::number((unsigned int)time.in[0],10);
				//qDebug()<<time.in[0]<<shortName<<ByteArray_to_String(a);
			}
		}
		fileName+=shortName;
		fileName+=afterName;
	}

	void seveFile(){
		QFile file; //файловые переменные. источник и назначение
		file.setFileName(fileName);
		if(file.open(QIODevice::WriteOnly)){;
			file.write(data);
			file.close();
			data.clear();
			rez=OK;
		}else rez=BAD;
	}

private:
	 //обьединение для данных (универсальное) на 8 байт
//#pragma pack(push, 1)
	union lon{
		unsigned long long int din;
		unsigned long int	   in[2];
		char			  	   cha[8];
		bool				   bo[64];
	};
//#pragma pack(pop)
};

class dBROADCAST : public dIF{
public:
	dBROADCAST(){
		Type=BROADCAST;
		wait=0;
	}
    virtual ~dBROADCAST(){}
};


class dMULTICAST : public dFOR{
public:

	QMap<quint32,QByteArray>   multiArray;//накопитель ответов для мультикаста/броадкаста

	dMULTICAST(){
		Type=MULTICAST;
		delay=5;
		ControlRez=true;
	}
	virtual ~dMULTICAST(){}
    int appendAdr(QString a,short m=-1){ udpAdr.append(a); mdbAdr.append(m); cou=udpAdr.size(); return cou;}
	QString getNextAdr(){
		if(cou>1) rez=CON;
		else      rez=OK;
		if(cou>0) if(mdbAdr[cou-1]>=0) cmd[0]=(unsigned char)mdbAdr[cou-1];
		return cou>0 ? udpAdr[udpAdr.size()-(cou--)] : QString();
	}


virtual bool test(const QByteArray& res){
	rezType r=rez;
	if(concommand::test(res)) multiArray[AddrRF.toIPv4Address()]=res;
	rez=r;
	return true;
}

protected:
	QStringList    udpAdr;      //кому шлем запроc (только udp)
	QList<short>   mdbAdr;

};



class dSETTIME : public dIF{   //функция и адрес генерятся автоматом - если не заданы
public:
	dSETTIME(){
		Type=SETTIME;
		setFun(80);
		msize=9;
		rez.resize(9);
	}
	virtual ~dSETTIME(){}
	virtual QByteArray getRES(){
		bool ok;
		rez.resize(9);
		if(cmd.size()==2){
			rez[0]=cmd[0];
			rez[1]=cmd[1];
			QString dt=dtime.currentDateTime().toString("hh:mm:ss:dd:MM:yy");
			rez[2]=(char)dt.section(':',0,0).toInt(&ok,10);
			rez[3]=(char)dt.section(':',1,1).toInt(&ok,10);
			rez[4]=(char)dt.section(':',2,2).toInt(&ok,10);
			rez[5]=(char)dt.section(':',3,3).toInt(&ok,10);
			rez[6]=(char)dt.section(':',4,4).toInt(&ok,10);
			rez[7]=(char)dt.section(':',5,5).toInt(&ok,10);
			rez[8]=20;
		}else{
			return dIF::getCmd();
		}
		return rez;
	}
protected:
	QDateTime  dtime;  	//время системное
	QByteArray rez;
};


class dCONNECT : public command{
public:
	 connectType   cType;
	 QString       cAddr;  //адрес/имя
	 QString       cParam; //номер слота или скорость порта
	 Prot485       cProt485;

	 QString       udpSAddr;      //где слушаем (только udp)
	 quint16	   udpSPort;      //на каком порту слушаем
	 execut*       extExe;

	 dCONNECT(){
		 Type=CONNECT;
		 cType=cEXT;
		 extExe=NULL;
		 cProt485=Modbus;
		 udpSPort=(int)rand() % 16383 + 49152;//откуда шлем для UDP;
		 udpSAddr="0.0.0.0"; //все
		 cAddr="0.0.0.0"; //все
	 }
	 virtual ~dCONNECT(){}
};
class dDISCONNECT : public command {
public:
	 dDISCONNECT(){
		 Type=DISCONNECT;
		 rez=OK;
		 delay=0;
	 }
	 virtual ~dDISCONNECT(){}

};


//typedef   scharm;

class DGSERVER_EXPORT tape: public QList<command*>
{
public:
	quint64     marker; //метка

	tape(quint64 m=0){
		marker=m;
		MaxRepeat=0;
		MaxTcpERRCoun=10;
		defWait=100;
	}
	virtual ~tape(){
		clear();
	}
	command* append(commandType,QString label=""); //инициализация записи шарманки с номером...и возврат ссылки не нее
	bool test(bool debug=false);   //вычисляет переходы и сообщает о результате
	void clear();

	void setMaxTcpERRCoun(quint32 n){ MaxTcpERRCoun=n;}
	int  getMaxTcpERRCoun()     { return MaxTcpERRCoun;}

	void setMaxRepeat(quint32 r){ MaxRepeat=r;}
	int  getMaxRepeat(){ return MaxRepeat;}

	void setDWait(uint w){defWait=w;}
	uint getDWait(){return defWait;}

protected:
	int find(QString l);
	int MaxRepeat;     //число повторов (com и UDP)
    int MaxTcpERRCoun;  //число повторов (TCP)
    uint defWait;
};


#endif /* TAPE_H_ */
