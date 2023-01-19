/*
 * snifferThread.cpp
 *
 *  Created on: 19 июня 2015 г.
 *      Author: Usach
 */

#include "snifferThread.h"



void snifferThread::run(){
	timer=new QTimer();
	timer->setSingleShot(true);
	activeTimer=new QTimer();


	QObject::connect(activeTimer, SIGNAL(timeout()), this, SLOT(testActive()));

    com=new QSerialPort(Addr);
    //QSerialPort com(Addr);		  //com порт
	com->setPortName(Addr);
	com->setBaudRate(Speed.toInt());
	/*
	com->setParity(QSerialPort::NoParity);
	com->setDataBits(QSerialPort::Data8);
	com->setStopBits(QSerialPort::OneStop);
	com->setFlowControl(QSerialPort::NoFlowControl);
	*/

	if (com->open(QIODevice::ReadOnly)) {
		emit message("Open port:"+Addr+":"+Speed);
		QObject::connect(com, SIGNAL(readyRead()), this, SLOT(Receive()));
		QObject::connect(timer, SIGNAL(timeout()), this, SLOT(breakCMDSlot()));
		activeTimer->setSingleShot(false);
		activeTimer->start(1000);
	}else{
		emit message(Addr+":"+Speed+ " "+com->errorString()+":"+QString::number(com->error()),1);
		activeTimer->setSingleShot(true);
		activeTimer->start(0);
	}
	exec();
    timer->stop();
	activeTimer->stop();
    if(com->isOpen())com->close();
 }

void snifferThread::stop(){
	quit();
}


void snifferThread::breakCMDSlot(){
	if(SYSTEM::Debug) debugD("Skip timeout",curM,CMD);
	uk=uk+8;
	breakCMD();
}

void snifferThread::breakCMD(){
	if(CMD){
		if(uk<buf.size()) memmove(buf.data(),buf.data()+uk,buf.size()-uk);
		buf.resize(buf.size()-uk);
		uk=0;
		CMD=0;
		curM=0;
	}
}

void snifferThread::debugD(const QString& txt,uchar M,uchar C){
	if(SYSTEM::NumDebug==curM || SYSTEM::NumDebug==0){
		emit message(txt+" "+Addr+":"+QString::number(M)+":"+QString::number(C));
	}
}

void snifferThread::Receive(){

	buf.append(com->readAll());
//	qDebug()<<buf.size();


	while(CMD==0 && buf.size()-uk>=8){
		CMD=getCommand(buf.data()+uk,lenData);
		if(!CMD){
			uk++;
			if(buf.size()>maxLen/2){
				memmove(buf.data(),buf.data()+uk,buf.size()-uk);
				buf.resize(buf.size()-uk);
				uk=0;
			}
		}else{
			curM=(unsigned char)buf[uk];
			timer->start(delay+1+(1000*lenData*8)/speedN);
		}
	}


	if(CMD!=0){
		if(buf.size()-uk>=lenData+8){
			timer->stop();
		//	qDebug()<<Array_to_String(buf.data()+uk,8)+"<-"+Array_to_String(buf.data()+uk+8,lenData);
			uk=uk+8;
			if(mList.contains(curM)){
				unsigned short cr=0;
				QVector<unsigned char> dat;
				dat.reserve(lenData-2);
				for(int i=uk;i<uk+lenData-2;i=i+2){
					UpdateCRC16((unsigned char)buf[i+1],&cr);
					UpdateCRC16((unsigned char)buf[i],&cr);
					dat<<(unsigned char)buf[i]<<(unsigned char)buf[i+1];
				}
				uk=uk+lenData-2;
				ucrc crc;
				crc.cha[0]=buf[uk+1];
				crc.cha[1]=buf[uk];
				uk=uk+2;
				if(crc.shor==cr) 	addData(curM,CMD,dat);
				else if(SYSTEM::Debug) debugD("Bad CRC data",curM,CMD);
			}else{
				uk=uk+lenData;
				if(SYSTEM::Debug) debugD("Not addr devise",curM,CMD);
			}
			breakCMD();
		}
	}


}

void snifferThread::addData(unsigned char M,unsigned char cmd,QVector<unsigned char> v){

	map->value(M)->updateData();//дата обновления
	if(map->value(M)->status!=MArray::eData){
		map->value(M)->status=MArray::eData;
		map->value(M)->dstatus=true;
		emit finStat(M);
		emit finDin(M);
		emit message("I heard data to:"+Addr+":"+QString::number(M));
	}
	switch(cmd){
	   case 1:
	   case 6:
		   	    map->command_x01(M,(const char*)v.data()); break;
	   case 2:  map->command_x02(M,(const char*)v.data()); break;
	   case 3:  map->command_x03(M,(const char*)v.data()); break;
	   case 4:  map->command_x04(M,(const char*)v.data()); break;
	   case 7:  map->command_x07(M,(const char*)v.data()); break;
	   case 9:  map->command_x09(M,(const char*)v.data()); break;
	   case 17: map->command_x11(M,(const char*)v.data()); break;
	   case 20: map->command_x14(M,(const char*)v.data()); break;
	   default:
		   if(SYSTEM::Debug) if(SYSTEM::Debug) debugD("Ignore command",M,cmd);
		   return;
	}
	 if(SYSTEM::Debug) debugD("Get command",M,cmd);
}

void snifferThread::testActive(){
	 short m;
	 if(com->isOpen()){
		 for(int i=0;i<mList.size(); i++){
			 m=mList.at(i);
			 if(QDateTime::currentMSecsSinceEpoch()-map->value(m)->endtime>timeD){
				 if(map->value(m)->status!=MArray::eInfo){
					 map->value(m)->status=MArray::eInfo;
					 map->value(m)->dstatus=false;
					 emit message("No data channel:"+Addr+":"+QString::number(m));
				 }
			 }
		 }
	 }else{
		 for(int i=0;i<mList.size(); i++){
			 m=mList.at(i);
				 map->value(m)->status=MArray::eInfo;
				 map->value(m)->dstatus=false;
		 }
		 activeTimer->stop();
		 emit message(QString("Not opened port:"+Addr),1);

	 }
}

char snifferThread::getCommand(char* cmd,int& len){
	ucrc crc,rl;
	len=0;

	crc.cha[0]=cmd[7];
	crc.cha[1]=cmd[6];
	if(crc.shor!=CRC16(cmd,3)) return 0;
	rl.cha[0]=cmd[3];
	rl.cha[1]=cmd[2];
	len=2*(rl.shor>>5);
	return cmd[1];
}


/*
 * Считаем CRC16
 */
unsigned short snifferThread::CRC16(char* d,int len){
	unsigned short cr;
	cr=0;
	for(int i=0;i<len;i++){
			UpdateCRC16((unsigned char)d[i*2+1],&cr);
			UpdateCRC16((unsigned char)d[i*2+0],&cr);
	}
	return cr;
}

void snifferThread::UpdateCRC16(unsigned char in, unsigned short *crc16)
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

QString snifferThread::Array_to_String(char* arr,int len){

	QString st,fst;
	for(int i=0;i<len;i++){
			//разбор байтов
			st.setNum(arr[i],16);
			if(st.size()<2) st="0"+st;
			st=st.right(2);
			fst.append(st);
	}
	return fst;
}

