/*
 * com485.cpp
 *
 *  Created on: 01 июля 2015 г.
 *      Author: Usach
 */

#include "com485.h"
#include <const.h>

uchar com485::debugN=0;


void com485::init(Prot485 prot){
	bsend.reserve(256);
	bresp.reserve(256);
	Prot=prot;
	lis=false;
	timer.setSingleShot(true);
	bytesWritten=0;
	lenLis=0;
	VektRez=false;
	wait=dWait;
	startCRC[Modbus]=0xFFFF;
	startCRC[Vector]=0x0000;
	crc16=startCRC[Prot];
	connect(this, SIGNAL(readyRead()), this, SLOT(readCOM()));
	connect(this, SIGNAL(bytesWritten(qint64)),this, SLOT(handleBytesWritten(qint64)));
	connect(this, SIGNAL(error(QSerialPort::SerialPortError)),this, SLOT(handleError(QSerialPort::SerialPortError)));
	connect(&timer, SIGNAL(timeout()), this,SLOT(handleTimeout()));
}



int com485::getLenR(uchar F,uchar PF,char* d){
	int len=-1;
	switch(F){//считаем размер ответа заранее, где можем
			case 0x14:
				len=3+(uchar)d[2]+2; break;
			case 0x65:
				switch(PF){
				 //  case 0x00: len=4;  break;
				 //  case 0x01: len=13; break;
				 //  case 0x02: len=13; break;
				   case 0x03: len=6+(uchar)d[5]+2;  break; //заранее неизв
				 //  case 0x0A: len=13; break;
				}
				break;
			case 0x68:
			case 0x69:
			case 0x6A:
				len=6+(uchar)d[5]+2;
				break;
		}
	return len;
}

int com485::getLenS(uchar F,uchar PF,char* d){
	int len=0;
	switch(F){//считаем размер ответа заранее, где можем
			case 0x03:
			case 0x04: 	len=3+(uchar)d[5]*2+2; break;
			case 0x50:  len=11;			break;
			case 0x41:
			case 0x43:  len=4+(uchar)d[3]*2+2; break;
			case 0x65:
				switch(PF){
				   case 0x00: len=4;  break;
				   case 0x01: len=13; break;
				   case 0x02: len=13; break;
				 //  case 0x03:   break; //заранее неизв
				   case 0x0A: len=13; break;
				}
				break;
			case 0x66: 	len=6;  break;
			case 0x67: 	len=13; break;
		}
	return len;
}

void com485::readCOM(){

	timer.stop();
	if(!lis){ readAll(); bresp.clear(); return;}
	bool crc=false;
	bool fErr=true;
	int start=0;
	int oldSz=bresp.size();

	bresp.append(readAll());

	if(oldSz==0) crc16=startCRC[Prot];
	if(crc16!=startCRC[Prot]) start=oldSz-2;

	if(Prot==Modbus){
		if(bresp.size()>3){
			if(fErr){
				if(F==(uchar)bresp.at(1)+0x80) lenLis=5;
				else fErr=false;
			}
			crc=TestAddCRC16Modbus((char*)(bresp.data()+start),bresp.size()-start,&crc16);
		}

		if(lenLis==0 && bresp.size()>=6){
			lenLis=getLenR(F,PF,bresp.data());
		}
	}

	if(lenLis>0){
		if(bresp.size()>=lenLis){
			if(Prot==Vector){
				crc=TestCRC16(bresp);
				VektRez=true;
			}
			resiveData(bresp,lenLis,crc);
		}
		else                     timer.start(wait);
	}else{
		if(crc)					 resiveData(bresp,bresp.size(),crc);
		else    				 timer.start(wait);
	}
}


int com485::getLenV(const QByteArray& arr){

	ucrc sz;
	if(arr.size()<6) return 0;
	sz.cha[0]=arr.at(3);
	sz.cha[1]=arr.at(2);
	return (sz.ushor>>5)*2;
}

void com485::dataWrite(const QByteArray& arr){
	timer.stop();
	VektRez=false;
	lis=false;
	bresp.clear();
	bsend.clear();
	lenLis=0;
	bytesWritten=0;

	bsend.append(arr);
	M=(uchar)bsend[0];
	F=(uchar)bsend[1];
	if(bsend.size()>2) PF=(uchar)bsend[2];
	if(Prot==Modbus) lenLis=getLenS(F,PF,bsend.data());
	else             lenLis=getLenV(bsend);

	AddCRC16(bsend);
//	qDebug()<<ByteArray_to_String(bsend);

    qint64 bytesW = write(bsend);
    if (bytesW == -1) {
    	emit errorStr(QString("Failed to write the data to port %1, error: %2").arg(portName()).arg(errorString()));
    	return;
    }
   // handleBytesWritten(bytesW);
    if(M!=0) timer.start(3000);  //не броадкаст
}

void com485::closeVector(){
	if(Prot==Vector){
		if(VektRez==false){
			write(QByteArray(lenLis-bresp.size(),0x00));
			//qDebug()<<"Cloze V:"<<lenLis-bresp.size();
		}
		VektRez=false;
	}
}

void com485::handleBytesWritten(qint64 bytes)
{
	bytesWritten += bytes;
    if (bytesWritten == bsend.size()) {
    	bytesWritten = 0;
    	//bsend.clear();
    	lis=true;
    }
}

void com485::handleTimeout()
{
	if(!lis) emit errorStr(QString("Operation write timed out for port %1, error: %2").arg(portName()).arg(errorString()));
	else{
		if(debugN==M)
			qDebug()<<"Time error->"+QString::number(wait);
		resiveData(bresp,bresp.size(),false);
	}
}

void com485::resiveData(QByteArray& arr,int len,bool ok){
	lis=false;
	if(arr.size()!=len) qDebug()<<"Size error!!!";
	if(!ok){
		if(debugN==M && arr.size()>0) qDebug()<<"CRC Bad:"+ByteArray_to_String(arr);
		emit dataRead(QByteArray());
	}else{
		emit dataRead(arr.left(len-2));//вернули данные
	}
	arr.clear();
}

void com485::handleError(QSerialPort::SerialPortError serialPortError)
{
    if (serialPortError == QSerialPort::WriteError) {
      emit errorStr(QString("An I/O error occurred while writing the data to port %1, error: %2").arg(portName()).arg(errorString()));
    }
    if (serialPortError == QSerialPort::ReadError) {
    	emit errorStr(QString("An I/O error occurred while reading the data from port %1, error: %2").arg(portName()).arg(errorString()));
    }
}


//---------------------------------------------------------------------

QString com485::ByteArray_to_String(const QByteArray& arr){

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

//*******************************************************

/*
 * Проверяем CRC16
 */
bool com485::TestCRC16(const QByteArray& arr){
	if(arr.size()<3) return false;
	QByteArray ret;
	ucrc crc;
	unsigned short cr;
	crc.cha[0]=arr.at(arr.size()-2);
	crc.cha[1]=arr.at(arr.size()-1);
	if(Prot==Modbus) cr=CRC16ModbusArr(arr,arr.size()-2);
	else			 cr=CRC16VectorArr(arr,arr.size()-2);

	return (crc.ushor==cr);
}

/*
 * проверяем добавляя
 */
bool com485::TestAddCRC16Modbus(const char* d,int len,unsigned short* crc){
	 if(len<2) return false;
	 ucrc cr;
	 cr.cha[0]=*(d+len-2);
	 cr.cha[1]=*(d+len-1);
	 for(int i=0;i<len-2;i++) UpdateCRC16Modbus(*((uchar*)(d+i)),crc);
	 return (*crc==cr.ushor);
}

/*
 * Возвращаем с добавленной CRC16
 */
void com485::AddCRC16(QByteArray& arr){
	ucrc crc;
	if(Prot==Modbus) crc.shor=CRC16ModbusArr(arr);
	else             crc.shor=CRC16VectorArr(arr);
	arr.append(crc.cha[0]);
	arr.append(crc.cha[1]);
}


//***************************************************************************
/*
 * Считаем CRC16 modbus
 */
unsigned short com485::CRC16ModbusArr(const QByteArray& array,int len){
	unsigned short crc16;
	if(len<2 || len>array.size()) crc16=CRC16Modbus(array.data(),array.size());
	else   crc16=CRC16Modbus(array.data(),len);
	return crc16;
}

unsigned short com485::CRC16Modbus(const char* d,int len){
	unsigned short crc16=0xFFFF;
	for(int i=0;i<len;i++){
		UpdateCRC16Modbus(d[i],&crc16);
	}
	return crc16;
}


/*
 * Функция обновления CRC16 входящим байтом. для modbus
 * Исходное состояние 0xFFFF
 */
void com485::UpdateCRC16Modbus(unsigned char in, unsigned short *crc16)
{//Функция обновления CRC16 входящим байтом. для modbus
unsigned short x,y,i;
	x=*crc16 ^ in;
	for(i=0;i<8;i++)
	{
      y=x>>1;
	  if ((x & 0x0001)==0) x=y;
	  else x=y ^ 0xA001;
	}//for
	*crc16=x; //Обновить
}//CRC_Update


//***************************************************************
/*
 * Считаем CRC16 Vector
 */

unsigned short com485::CRC16VectorArr(const QByteArray& array,int len){
	unsigned short crc16;
	if(len<2 || len>array.size()) crc16=CRC16Vector(array.data(),array.size());
	else   crc16=CRC16Vector(array.data(),len);
	return crc16;
}


unsigned short com485::CRC16Vector(const char* d,int len){
	unsigned short rez,cr=0;
	for(int i=0;i<len/2;i++){
		UpdateCRC16Vector((unsigned char)d[i*2+1],&cr);
		UpdateCRC16Vector((unsigned char)d[i*2+0],&cr);
	}
	rez=cr<<8;
	rez+=cr>>8;
	return rez;
}

void com485::UpdateCRC16Vector(unsigned char in, unsigned short *crc16)
{//Функция обновления CRC16 входящим байтом для вектор
//Байты необходимо закладывать по принципу, сначала младший, потом старший.
short i,x,y;
	x=*crc16 ^ (in<<8);
	for(i=0;i<8;i++)
	{
      y=x<<1;
	  if ((x & 0x8000)==0) x=y;
	  else x=y ^ 0x1021;
	}//for
	*crc16=x; //Обновить
}//CRC_Update


