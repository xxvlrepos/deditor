/*
 * comport.cpp
 *
 *  Created on: 01.12.2009
 *      Author: Usach
 */

#include "ipport.h"
#include <QtCore>



ipport::ipport() {

	port=23;
	adr="127.0.0.1";
	codec = QTextCodec::codecForName("IBM 866"); //установка перекодировщика
    QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(sendRead()));
    QObject::connect(&stimer, SIGNAL(timeout()), this, SLOT(sendBuf()));
    QObject::connect(&socket, SIGNAL(readyRead()), this, SLOT(receiveMsg()));
    QObject::connect(&socket, SIGNAL(connected()), this, SLOT(ipConnect()));
    QObject::connect(&socket, SIGNAL(disconnected()),this, SLOT(ipDisconnect()));
    QObject::connect(&socket, SIGNAL(bytesWritten(qint64)),this, SLOT(Writen(qint64)));
    putCou=0;
    wriCou=0;
    Desyn=0;
    debug=false;
}

ipport::~ipport() {
	// TODO Auto-generated destructor stub
}

/*
 * Слот.
 * Данные отправлены
 * в случае рассинхронизации - сигналим о величине пока не 0
 */
void ipport::Writen(qint64 bit){
	wriCou+=bit;
	int ds=putCou-wriCou;
	if(ds>0 || Desyn>0) emit desync(ds);
	Desyn=ds;

	if(debug){
		emit debugSend((int)bit);
		//emit debugTime(taut.elapsed());
		/*
		QString ss,str,str1;
		ss.setNum((int)bit,10);
		qDebug()<<ss;
		str.setNum(putCou-wriCou,10);
		qDebug()<<"desync: "+str;
		*/
	}
}

/*
 * Вкл./выкл отладки
 */
void ipport::setDebug(bool dd){
	debug=dd;
}

/**
* Перехватывающие слоты
*/
void ipport::ipConnect(){
emit Open(true);
}

void ipport::ipDisconnect(){
emit Open(false);
}

/* Слот
 * Открыть порт
 */
bool ipport::openPort() {
   socket.connectToHost(adr,port);
   if(!socket.waitForConnected(2000)){
		socket.disconnectFromHost();
		emit Open(false);
		return false;
   }else{
	    emit debugSig("slot open");
	    emit Open(true);
	    return true;
   }
}

/* Слот
 * Открыть порт
 */
bool ipport::openPort(bool ck) {
	if(ck) return openPort();
	else closePort();
	return false;
}

/* Слот
 * Закрыть порт
 */
void ipport::closePort() {
	socket.disconnectFromHost();
	emit debugSig("slot close");
}


/* private
 * Читает порт в буфер кусками rbuf
 */
void ipport::receiveMsg() {
	char ch;
	QByteArray buff = socket.readAll();
	if(debug){
		emit debugTime(taut.elapsed());
		emit debugRead(buff.size());
	}
	emit replyFArr(buff); //сигнал без задержки
	b_raw_read.append(buff);
	int i=buff.size();
	for (int j=0; j<i; j++) {
		//излучаем символ
		ch=buff.at(j);
		emit replyCh(ch);
		//можно все сразу слить, но приходится контролировать нули для строк
		if(ch!='\0') b_read.append(ch);
		else b_read.append(' ');
	}
	if(b_read.size()>=rbuf)	 sendRead();
	timer.start(delay);
	if(debug) taut.start();
}

/* Слот
 * Передает строку в порт с завершающим символом
 */
void ipport::putMsg(const QString &message) {
	  transmitMsg(message);
}

/* public
 * Передает строку в порт с завершающим символом
 */
bool ipport::transmitMsg(const QString &message) {

	if(!socket.isOpen()) {
		emit debugSig("slot close");
		return false;
		}
	else {
		QByteArray mes=message.toAscii();
		mes.append(end_ch);
		//все русские - в прописные
		for(int j=0;j<mes.length();j++){
			if((unsigned char)mes[j]>=0xE0 && (unsigned char)mes[j]<=0xEF) mes[j]=mes[j]-0x50;
			if((unsigned char)mes[j]>=0xA0 && (unsigned char)mes[j]<=0xAF) mes[j]=mes[j]-0x20;
		}
		putCou+=socket.write(mes,mes.length());
		if(debug) taut.start();
		return true;
	}
}

/* public
 * Передает массив в порт с завершающим символом
 */
bool ipport::transmitMsg(const QByteArray &message) {
	QString msg=message;
	return transmitMsg(msg);
}



/*
 * Слот
 * посылает в порт накопленный буфер по таймауту
 */
void ipport::sendBuf(){
	if(socket.isOpen()){
		 stimer.stop();
		 if(debug) taut.start();
		 socket.flush();
	}
}

/* Слот
 * Символ в порт
 */
void ipport::putPort(const char &ch){
	 putCou+=socket.write(&ch,1);
	 stimer.start(sdelay);
}



/* Слот
 * строку в порт
 */
void ipport::putPort(const QString &st){
	if(socket.isOpen()){
	 putCou+=socket.write(st.toAscii(),st.size());
	 if(debug) taut.start();
	}
}

/* Слот
 * битовый массив в порт
 */
void ipport::putPort(const QByteArray &st){
	if(socket.isOpen()){
	 putCou+=socket.write(st,st.size());
	 if(debug) taut.start();
	}
}

/* Слот
 * Выплевываем все из буфера
 */
void ipport::sendRead() {
	//выплевываем, если есть что и очищаем буфера
	if(b_raw_read.size()>0){
		emit replyArr(b_raw_read);
		b_raw_read.clear();
	}
	if(b_read.size()>0){
		emit replyMsg(codec->toUnicode(b_read));
		b_read.clear();
		//длинна
		emit debugRead(b_read.length());
	}
}

/** Слот
 *  Меняем порт с разрывом связи
 */
void ipport::setIpPort(const QString &name) {
  	adr=name;
	closePort();
}

/**
 *
 */
void ipport::sendPort(){
	if(sendMsg!="")  transmitMsg(sendMsg);
}
