/*
 * mMap.cpp
 *
 *  Created on: 24 июня 2014 г.
 *      Author: Usach
 */

#include "mMap.h"

//int mMap::Repeat=0;  //число повторов при потере
int mMap::TimeOut=150;  //ждем ответ
mMap* mMap::mdbMap=NULL;

QByteArray MArray::midM( int pos, int len ){
		mutex.lock();
		const QByteArray& rez=mid(pos,len);
		mutex.unlock();
		return rez;
}
void MArray::replaceM(int pos,const char* a,unsigned char alen){
	if(pos>=dSize) return;
	if(pos+alen>dSize) alen=dSize-pos;
	mutex.lock();
	   memcpy((char*)(data()+pos),a,alen);
	mutex.unlock();
}
void MArray::replaceM(int pos,const QByteArray & a){
	replaceM(pos,a.data(),a.size());
}

void MArray::setMData(int ma, short d){
	usort u;
	if(ma<Shift || ma>=maxBAdr) return;
	int a=2*(ma-Shift);
	u.sd=d;
	mutex.lock();
		*(data()+a)  =u.sc[1];
		*(data()+a+1)=u.sc[0];
	mutex.unlock();
}

short MArray::getMData(int ma){
	usort d;
	if(ma<Shift || ma>=maxBAdr) return 0;
	int a=2*(ma-Shift);
	mutex.lock();
	d.sc[1]=*(data()+a);
	d.sc[0]=*(data()+a+1);
	mutex.unlock();
	return d.sd;
}

QVector<short> MArray::getMData(int ma,int len){
	if(ma<Shift || ma>=maxBAdr) return QVector<short>();
	if(ma+len>maxBAdr) len=maxBAdr-ma;
	int a=2*(ma-Shift);
	QVector<short> rez(len);
	mutex.lock();
	for(int i=0; i<len;i++){
		*((char*)rez.data()+2*i+1)=*(data()+a);
		*((char*)rez.data()+2*i)  =*(data()+a+1);
		a+=2;
	}
	mutex.unlock();
	return rez;
}


void MArray::setBad(QByteArray& res,unsigned char bad){
		   res.resize(9);
		   res[5]=3;
		   res[7]=res.at(7)+0x80;
		   res[8]=bad;
}



void MArray::fun04(QByteArray& com,QByteArray& res){
	   int ad,le;
	   if(com.size()!=12)  { setBad(res,0x03); return;}
	   le=com[11];
	   ad=(unsigned char)com[8];
	   ad=(ad<<8)+(unsigned char)com[9];
	   if(le<1 || le>0x7D) { setBad(res,0x03); return;}
	   if(ad<Shift || ad>=maxBAdr) { setBad(res,0x02); return;}
	   res.append(le*2);
	   res.append(midM(2*(ad-Shift),2*le));
	   res[5]=le*2+3;

}




 //******************************************************************************************


void  mMap::clear(){
	 QList<unsigned char> k=keys();
	 for(int i=0; i<k.size();i++)   delete(value(k[i]));
	 QHash<unsigned char,MArray*>::clear();
}

void mMap::updateData(const QByteArray& sr,const QByteArray& re){

	MArray* m;
	int lo,al,n;
	unsigned char len;
	unsigned char ma=(unsigned char)sr[0];
	unsigned char cmd=sr[1];
	if(contains(sr[0])) m=value(ma);
	else 				return;


	if(m->type==MArray::kNONE){//Modbus
		if(sr[1]!=re[1]) return;
		switch(cmd){
			case 0x04:           //afiiiixxxx
				m->replaceM((int)getMArrData(sr,2)*2,re.data()+3,(unsigned char)re[2]);
				break;
			case 0x14:        //af0L6ddL6dd
				lo=5;         //0123456789
				al=3;
				n=((unsigned char)sr[2])/7;
				for(int i=0;i<n;i++){
					len=(unsigned char)re[al]-1;
					m->replaceM(2*getMArrData(sr,6+i*7),re.data()+lo,len);
					lo+=len+2;
					al+=len+2;
				}
				break;
			case 0x2B://версия прошивки

#if QT_VERSION < 0x050000
				m->ver=QString::fromAscii((re.data()+9),(int)re[8]);
#else
				m->ver=QString::fromLatin1((re.data()+9),(int)re[8]);
#endif
				QLocale loc(QLocale::English, QLocale::UnitedStates);
				QDateTime d=loc.toDateTime(m->ver,"MMM dd yyyy");
				m->vdat=d.date();
				break;
		}
	}else{ //Vector
		switch(cmd){
			   case 1:
			   case 6:
				   	    command_x01(ma,re.data()); break;
			   case 2:  command_x02(ma,re.data()); break;
			   case 3:  command_x03(ma,re.data()); break;
			   case 4:  command_x04(ma,re.data()); break;
			   case 7:  command_x07(ma,re.data()); break;
			   case 9:  command_x09(ma,re.data()); break;
			   case 17: command_x11(ma,re.data()); break;
			   case 20: command_x14(ma,re.data()); break;
		}
	}
}

short mMap::getMArrData(const QByteArray& sr,int n){
	usort d;
	if(n>=sr.size()) return 0;
	d.sc[1]=*(sr.data()+n);
	d.sc[0]=*(sr.data()+n+1);
	return d.sd;
}

QMap<unsigned char,int> mMap::getStatus(){
			QMap<unsigned char,int> rez;
			mMap::const_iterator im=this->begin();
			for(;im!=this->end();im++) rez[im.key()]=im.value()->status;
			return rez;
	}
QMap<unsigned char,bool> mMap::getDStatus(){
			QMap<unsigned char,bool> rez;
			mMap::const_iterator im=this->begin();
			for(;im!=this->end();im++) rez[im.key()]=im.value()->dstatus;
			return rez;
}

//*********************************************************************************
//Vector
//*********************************************************************************

void mMap::command_x01(unsigned char m, const char* dat){
	MArray* dm=value(m);
	switch(dm->type){
		case MArray::kA: dm->replaceM(0x0001*2,dat,2); break;//Ve
		case MArray::kT: dm->replaceM(0x0402*2,dat,2); break;//T
		case MArray::kC: dm->replaceM(0x001E*2,dat,2); break;//S
		case MArray::kD: dm->replaceM(0x001D*2,dat,2); break;//Z
		case MArray::kI: dm->replaceM(0x001E*2,dat,2); break;//I
	}
}

void mMap::command_x02(unsigned char m,const char* dat){
	MArray* dm=value(m);
	unsigned char ch,mas=0;
	unsigned char     no=0;
	switch(dm->type){
		case MArray::kT: dm->replaceM(0x0402*2,(char*)dat,2); break;//T
		case MArray::kC:
			dm->replaceM(0x001E*2,(char*)(dat+0*2),2);//S
			dm->replaceM(0x001D*2,(char*)(dat+1*2),2);//Z
			dm->replaceM(0x0056*2,(char*)(dat+2*2),2);//обороты 1 тах
			break;
		case MArray::kD: dm->replaceM(0x001D*2,(char*)dat,2); break;//Z
		case MArray::kI:
			dm->replaceM(0x001E*2,(char*)(dat+0*2),2);//S
			dm->replaceM(0x001D*2,(char*)(dat+1*2),2);//Z
			dm->replaceM(0x0056*2,(char*)(dat+2*2),2);//обороты 1 тах
			break;
	}
	//сработка уставок
	switch(dm->type){
			case MArray::kC:
				ch=dat[3*2+1];  //последний
				if(ch & 0x01) no|=0x02;   //ERR (9)
				if(ch & 0x02) mas|=0x20;  //H1  (5)
				if(ch & 0x04) mas|=0x40;  //H2  (6)
				if(ch & 0x08) mas|=0x80;  //H3  (7)
				dm->replaceM(0x10F1*2,  (char*)&no,1);
				dm->replaceM(0x10F1*2+1,(char*)&mas,1);
				break;
			case MArray::kI:
				ch=dat[3*2+1];  //последний
				if(ch & 0x01) no|=0x02;   //ERR (9)
				if(ch & 0x02) mas|=0x20;  //H1_1
				dm->replaceM(0x10F1*2,  (char*)&no,1);
				dm->replaceM(0x10F1*2+1,(char*)&mas,1);
				mas=0;
				if(ch & 0x04) mas|=0x20;  //H1_2
				dm->replaceM(0x10F2*2,  (char*)&no,1);
				dm->replaceM(0x10F2*2+1,(char*)&mas,1);
				break;
			case MArray::kD:
			case MArray::kT:
				ch=dat[3*2+1];  //последний
				if(ch & 0x01) no|=0x02;   //ERR (9) //только обрыв - остальное неодназначно
				dm->replaceM(0x10F1*2,  (char*)&no,1);
				dm->replaceM(0x10F1*2+1,(char*)&mas,1);
				break;

	}

}



void mMap::command_x03(unsigned char m,const char* dat){
	MArray* dm=value(m);
	switch(dm->type){
		case MArray::kA:
			dm->replaceM(0x01*2,(char*)(dat+0*2),2); //Ve
			dm->replaceM(0x05*2,(char*)(dat+1*2),2*4); //V1,Fz1,V2,Fz2
			dm->replaceM(0x02*2,(char*)(dat+5*2),2); //NVe
			dm->replaceM(0x46*2,(char*)(dat+6*2),2*7);//V3,V4...V9

			ucrc ch;
			ch.cha[1]=dat[15*2];
			ch.cha[0]=dat[15*2+1];
			ch.shor/=10;
			dm->replaceM(0x03*2,  &ch.cha[1],1);   //Spp
			dm->replaceM(0x03*2+1,&ch.cha[0],1); //Spp
			break;
		case MArray::kC:
			dm->replaceM(0x001E*2,(char*)(dat+0*2),2); //S
			dm->replaceM(0x0020*2,(char*)(dat+1*2),2*4); //S1,Fz1,S2,Fz2
			dm->replaceM(0x004D*2,(char*)(dat+6*2),2*5); //S3...S7
			dm->replaceM(0x001D*2,(char*)(dat+13*2),2); //Z
			break;
		case MArray::kD: dm->replaceM(0x001D*2,(char*)dat,2); break;//Z
	}
}

void mMap::command_x04(unsigned char m,const char* dat){
	MArray* dm=value(m);
	unsigned char ch,mas=0;
	unsigned char     no=0;
	switch(dm->type){
		case MArray::kA:
			dm->replaceM(0x01*2,(char*)(dat+0*2),2); //Ve
			dm->replaceM(0x05*2,(char*)(dat+1*2),2*4); //V1,Fz1,V2,Fz2
			dm->replaceM(0x02*2,(char*)(dat+5*2),2); //NVe
			dm->replaceM(0x03*2,(char*)(dat+6*2),2); //Spp
			dm->replaceM(0x56*2,(char*)(dat+7*2),2); //F1  ADC
			dm->replaceM(0x402*2,(char*)(dat+7*2),2); //F1 T1
			break;
		case MArray::kC:
			dm->replaceM(0x001E*2,(char*)(dat+0*2),2); //S
			dm->replaceM(0x0020*2,(char*)(dat+1*2),2*4); //S1,Fz1,S2,Fz2
			dm->replaceM(0x001D*2,(char*)(dat+6*2),2); break;//Z
			dm->replaceM(0x56*2,(char*)(dat+7*2),2); //F1
			break;
	}
	//сработка уставок
	switch(dm->type){
	    case MArray::kA:
		case MArray::kC:
			ch=dat[8*2+1];  //последний
			if(ch & 0x01) no|=0x02;   //ERR (9)
			if(ch & 0x02) mas|=0x20;  //H1  (5)
			if(ch & 0x04) mas|=0x40;  //H2  (6)
			if(ch & 0x08) mas|=0x80;  //H3  (7)
			dm->replaceM(0x10F1*2,  (char*)&no,1);
			dm->replaceM(0x10F1*2+1,(char*)&mas,1);
			break;
	}
}
void mMap::command_x07(unsigned char m,const char* dat){
	MArray* dm=value(m);
	unsigned char ch,mas=0;
	unsigned char     no=0;
	switch(dm->type){
		case MArray::kA:
			dm->replaceM(0x01*2,(char*)(dat+0*2),2); //Ve
			dm->replaceM(0x05*2,(char*)(dat+1*2),2*4); //V1,Fz1,V2,Fz2
			dm->replaceM(0x02*2,(char*)(dat+5*2),2); //NVe
			dm->replaceM(0x46*2,(char*)(dat+6*2),2*7);//V3,V4...V9
			dm->replaceM(0x03*2,(char*)(dat+15*2),2); //Spp
			dm->replaceM(0x56*2, (char*)(dat+16*2),2);  //F1  ADC
			dm->replaceM(0x402*2,(char*)(dat+16*2),2); //F1 T1
			break;
		case MArray::kC:
			dm->replaceM(0x001E*2,(char*)(dat+0*2),2); //S
			dm->replaceM(0x0020*2,(char*)(dat+1*2),2*4); //S1,Fz1,S2,Fz2
			dm->replaceM(0x004D*2,(char*)(dat+6*2),2*5); //S3...S7
			dm->replaceM(0x001D*2,(char*)(dat+11*2),2); //Z
			dm->replaceM(0x56*2,(char*)(dat+12*2),2); //F1
			break;
	}
	//сработка уставок
	switch(dm->type){
		case MArray::kA:
		case MArray::kC:
			ch=dat[13*2+1];  //последний
			if(ch & 0x01) no|=0x02;   //ERR (9)
			if(ch & 0x02) mas|=0x20;  //H1  (5)
			if(ch & 0x04) mas|=0x40;  //H2  (6)
			if(ch & 0x08) mas|=0x80;  //H3  (7)
			dm->replaceM(0x10F1*2,  (char*)&no,1);
			dm->replaceM(0x10F1*2+1,(char*)&mas,1);
			break;
	}



}
void mMap::command_x09(unsigned char m,const char* dat){
	unsigned char ch,mas=0;
	char          no=0;
	MArray* dm=value(m);
switch(dm->type){
		case MArray::kA:
			dm->replaceM(0x10*2,(char*)(dat+0*2),2);  //Hr
			dm->replaceM(0x0F*2,(char*)(dat+1*2),2);  //H3
			dm->replaceM(0x0E*2,(char*)(dat+2*2),2);  //H2
			dm->replaceM(0x0D*2,(char*)(dat+3*2),2);  //H1
			dm->replaceM(0x0C*2,(char*)(dat+4*2),2);  //L1
			dm->replaceM(0x0B*2,(char*)(dat+5*2),2);  //L2
			dm->replaceM(0x0A*2,(char*)(dat+6*2),2);  //L3
			dm->replaceM(0x09*2,(char*)(dat+7*2),2);  //Lr
			break;
		case MArray::kC:
			dm->replaceM(0x2D*2,(char*)(dat+0*2),2);  //Hr
			dm->replaceM(0x2C*2,(char*)(dat+1*2),2);  //H3
			dm->replaceM(0x2B*2,(char*)(dat+2*2),2);  //H2
			dm->replaceM(0x2A*2,(char*)(dat+3*2),2);  //H1
			dm->replaceM(0x29*2,(char*)(dat+4*2),2);  //L1
			dm->replaceM(0x28*2,(char*)(dat+5*2),2);  //L2
			dm->replaceM(0x27*2,(char*)(dat+6*2),2);  //L3
			dm->replaceM(0x26*2,(char*)(dat+7*2),2);  //Lr
			break;
		case MArray::kD:
			dm->replaceM(0x45*2,(char*)(dat+0*2),2);  //Hr
			dm->replaceM(0x44*2,(char*)(dat+1*2),2);  //H3
			dm->replaceM(0x43*2,(char*)(dat+2*2),2);  //H2
			dm->replaceM(0x42*2,(char*)(dat+3*2),2);  //H1
			dm->replaceM(0x41*2,(char*)(dat+4*2),2);  //L1
			dm->replaceM(0x40*2,(char*)(dat+5*2),2);  //L2
			dm->replaceM(0x3F*2,(char*)(dat+6*2),2);  //L3
			dm->replaceM(0x3E*2,(char*)(dat+7*2),2);  //Lr
			break;
		case MArray::kI:
			dm->replaceM(0x2D*2,(char*)(dat+0*2),2);  //Hr
			dm->replaceM(0x2A*2,(char*)(dat+3*2),2);  //H1_1 в размах
			dm->replaceM(0x3A*2,(char*)(dat+2*2),2);  //H1_2 в перемещ
			dm->replaceM(0x26*2,(char*)(dat+7*2),2);  //Lr
			break;
		case MArray::kT:
			dm->replaceM(0x040A*2,(char*)(dat+0*2),2);  //Hr
			dm->replaceM(0x0409*2,(char*)(dat+1*2),2);  //H3
			dm->replaceM(0x0408*2,(char*)(dat+2*2),2);  //H2
			dm->replaceM(0x0407*2,(char*)(dat+3*2),2);  //H1
			dm->replaceM(0x0406*2,(char*)(dat+4*2),2);  //L1
			dm->replaceM(0x0405*2,(char*)(dat+5*2),2);  //L2
			dm->replaceM(0x0404*2,(char*)(dat+6*2),2);  //L3
			dm->replaceM(0x0403*2,(char*)(dat+7*2),2);  //Lr
			break;

	}
	switch(dm->type){
		case MArray::kA:
		case MArray::kC:
		case MArray::kD:
			dm->replaceM(0x1007*2,(char*)(dat+0*2),2);  //Hr
			dm->replaceM(0x1006*2,(char*)(dat+1*2),2);  //H3
			dm->replaceM(0x1005*2,(char*)(dat+2*2),2);  //H2
			dm->replaceM(0x1004*2,(char*)(dat+3*2),2);  //H1
			dm->replaceM(0x1003*2,(char*)(dat+4*2),2);  //L1
			dm->replaceM(0x1002*2,(char*)(dat+5*2),2);  //L2
			dm->replaceM(0x1001*2,(char*)(dat+6*2),2);  //L3
			dm->replaceM(0x1000*2,(char*)(dat+7*2),2);  //Lr

			ch=dat[8*2+1];  //последний
			if(ch & 0x01) mas|=0x80;
			if(ch & 0x02) mas|=0x40;
			if(ch & 0x04) mas|=0x20;
			if(ch & 0x08) mas|=0x10;
			if(ch & 0x10) mas|=0x08;
			if(ch & 0x20) mas|=0x04;
			if(ch & 0x40) mas|=0x02;
			if(ch & 0x80) mas|=0x01;
			dm->replaceM(0x1008*2,  (char*)&no,1);
			dm->replaceM(0x1008*2+1,(char*)&mas,1);
			break;
		case MArray::kI:
			dm->replaceM(0x1007*2,(char*)(dat+0*2),2);  //Hr
			dm->replaceM(0x1004*2,(char*)(dat+3*2),2);  //H1_1
			dm->replaceM(0x1000*2,(char*)(dat+7*2),2);  //Lr
			dm->replaceM(0x100E*2,(char*)(dat+2*2),2);  //H1_2

			ch=dat[8*2+1];
			if(ch & 0x01) mas|=0x80;          //Hr
		//  if(ch & 0x02) mas|=0x40;
		//  if(ch & 0x04) mas|=0x20;          //H1_2
			if(ch & 0x08) mas|=0x10;          //H1_1
		//	if(ch & 0x10) mas|=0x08;
		//	if(ch & 0x20) mas|=0x04;
		//  if(ch & 0x40) mas|=0x02;
			if(ch & 0x80) mas|=0x01;          //Lr
			dm->replaceM(0x1008*2,  (char*)&no,1);
			dm->replaceM(0x1008*2+1,(char*)&mas,1); //как первый параметр
			mas=0;
			if(ch & 0x04) mas|=0x10;          //H1_2
			dm->replaceM(0x1012*2,  (char*)&no,1);
			dm->replaceM(0x1012*2+1,(char*)&mas,1); //как второй параметр

			break;
		case MArray::kT:
			dm->replaceM(0x1407*2,(char*)(dat+0*2),2);  //Hr
			dm->replaceM(0x1406*2,(char*)(dat+1*2),2);  //H3
			dm->replaceM(0x1405*2,(char*)(dat+2*2),2);  //H2
			dm->replaceM(0x1404*2,(char*)(dat+3*2),2);  //H1
			dm->replaceM(0x1403*2,(char*)(dat+4*2),2);  //L1
			dm->replaceM(0x1402*2,(char*)(dat+5*2),2);  //L2
			dm->replaceM(0x1401*2,(char*)(dat+6*2),2);  //L3
			dm->replaceM(0x1400*2,(char*)(dat+7*2),2);  //Lr

			ch=dat[8*2+1];
			if(ch & 0x01) mas|=0x80;
			if(ch & 0x02) mas|=0x40;
			if(ch & 0x04) mas|=0x20;
			if(ch & 0x08) mas|=0x10;
			if(ch & 0x10) mas|=0x08;
			if(ch & 0x20) mas|=0x04;
			if(ch & 0x40) mas|=0x02;
			if(ch & 0x80) mas|=0x01;
			dm->replaceM(0x1408*2,  (char*)&no,1);
			dm->replaceM(0x1408*2+1,(char*)&mas,1);
			break;
	}



}
void mMap::command_x11(unsigned char m,const char* dat){
	MArray* dm=value(m);
	if(dm->type==MArray::kA){
			dm->replaceM(0x04*2,(char*)dat,2); //PfVe
	}
}
void mMap::command_x14(unsigned char m,const char* dat){
	MArray* dm=value(m);
	if(dm->type==MArray::kA){
		dm->replaceM(0x04*2,(char*)(dat+0*2),2); //Ve
		dm->replaceM(0x05*2,(char*)(dat+1*2),2*4); //V1,Fz1,V2,Fz2
		dm->replaceM(0x02*2,(char*)(dat+5*2),2); //NVe
		dm->replaceM(0x03*2,(char*)(dat+6*2),2); //Spp
		dm->replaceM(0x56*2,(char*)(dat+7*2),2); //F1  ADC
		dm->replaceM(0x402*2,(char*)(dat+7*2),2); //F1 T1
	}
}

