/*
 * info.cpp
 *
 *  Created on: 17.10.2012
 *      Author: Usach
 */

//":/templ/resource/template.html"

#include "info.h"

info::info(QString tmpl) {

	// TODO Auto-generated constructor stub
   form=NULL;
   rep=NULL;

	QFile file;
	file.setFileName(tmpl);
	if(!file.open(QFile::ReadOnly | QFile::Text)){
		qDebug()<<"not file:"+tmpl;
		return;
	}
	ftxt = QString::fromUtf8(file.readAll());
	file.close();

	form= new former(&ptxt);
	rep = new report();
}

info::~info() {
	if(form) delete(form);
	if(rep) delete(rep);
	// TODO Auto-generated destructor stub
}

void info::ViewGFG(QString cfg){
	 if(form==NULL || rep==NULL) return;
	 if(!arrs.ReadCFG(cfg)) return;

	// QString::SectionFlag flag = QString::SectionSkipEmpty;


	 if(arrs.contains("GEFILE")){
		 if(arrs["GEFILE"].p.count()>0){
			 QString name,path;
			 name=arrs["GEFILE"].p.at(0);
			 path=cfg.section('/',0,-2);
			 arrg.ReadGE(path+"/"+name);
		 }
	 }

	 ptxt.clear();
     if(arrs.count()>0) ParseForm(&ftxt);
 	 else return;

    bool open=false;
   	if(rep){
     		if(rep->isVisible()) open=true;
     		else rep->close();
   	}
   //	if(!open) rep = new report(this);
   	rep->set_String(&ptxt);
   	rep->Reload();
   	if(!open) {
   			rep->resize(900,600);
   			rep->show();
	} else {
   			rep->activateWindow();
	}
}

/**
 * Обработка шаблона отчета
 */
bool info::ParseForm(QString* fstr){

	QString st=""; //рабочая строка -номер
	QString sn=""; //рабочая строка- имя
	QString txt,tx,tx1,tx2,tx3; //временные строки
	int ia=1; //число параметров в таблице А
	int ic=1; //число параметров в таблице С
	bool ok;
	QString z="-"; //заполнитель пустоты
	int np=0;

	//работаем с шаблоном
	form->set_txt("templ",fstr);
	//извлекаем тела и заголовоки таблиц
	form->set_block("templ","A_ADD","A_ADD_VAR");
	form->set_block("templ","A_BODY","A_BODY_VAR");
	form->set_block("templ","A_HEAD","A_HEAD_VAR");
	form->set_block("templ","C_ADD","C_ADD_VAR");
	form->set_block("templ","C_BODY","C_BODY_VAR");
	form->set_block("templ","C_HEAD","C_HEAD_VAR");
	form->set_block("templ","D_BODY","D_BODY_VAR");
	form->set_block("templ","D_HEAD","D_HEAD_VAR");
	form->set_block("templ","V_BODY","V_BODY_VAR");
	form->set_block("templ","V_HEAD","V_HEAD_VAR");
	form->set_block("templ","T_BODY","T_BODY_VAR");
	form->set_block("templ","T_HEAD","T_HEAD_VAR");
	form->set_block("templ","GE_BODY","GE_BODY_VAR");
	form->set_block("templ","GE_HEAD","GE_HEAD_VAR");
	//заменяем неизвестные на прочерки
	form->get_undefined("A_BODY",z);
	form->get_undefined("C_BODY",z);
	form->get_undefined("D_BODY",z);
	form->get_undefined("V_BODY",z);
	form->get_undefined("T_BODY",z);
	form->get_undefined("templ",""); //остатки переменных
	//выходы
	form->set_block("templ","DAC_HEAD","DAC_HEAD_VAR");
	form->set_block("templ","DAC1_S","DAC1_S_VAR");
	form->set_block("templ","DAC2_S","DAC2_S_VAR");
	form->set_block("templ","DAC3_S","DAC3_S_VAR");
	form->set_block("templ","DAC4_S","DAC4_S_VAR");
	form->set_block("templ","DAC5_S","DAC5_S_VAR");
	form->set_block("templ","DAC6_S","DAC6_S_VAR");
	form->set_block("templ","DAC7_S","DAC7_S_VAR");
	//реле
	form->set_block("templ","RE_HEAD","RE_HEAD_VAR");
	form->set_block("templ","RE1_S","RE1_S_VAR");
	form->set_block("templ","RE2_S","RE2_S_VAR");
	form->set_block("templ","RE3_S","RE3_S_VAR");
	//пустое место - обьединение ячеек
	form->set_var("A_ROW","ROWSPAN="+tx1.setNum(ia),false);
	form->set_var("A_ADD_VAR","",false);
	form->set_var("C_ROW","ROWSPAN="+tx2.setNum(ic),false);
	form->set_var("C_ADD_VAR","",false);
	form->set_var("DAC_C","COLSPAN=1",false);
	form->set_var("RE_C","COLSPAN=1",false);
	form->set_var("END_C","COLSPAN=3",false);

	//групповые события

	if(arrg.count()>0){
		for(int i=0;i<sge.size();i++){
			if(arrg.contains(sge.at(i))){
			   form->set_var("N",sge.at(i),false);
			   form->set_var("GE_Name",arrg[sge.at(i)].comm,false);
			   form->set_var("Exp","",false);//далее будем разделять операнды пробелами  или переносами
			   for(int j=0;j<arrg[sge.at(i)].age.count();j++){
				   if(arrg[sge.at(i)].age.at(j)=="&") form->set_var("Exp","&amp;",true);
				   else form->set_var("Exp",arrg[sge.at(i)].age.at(j),true);
				   form->set_var("Exp","<wbr>",true);
			   }
			   form->parse("GE_BODY_VAR","GE_BODY",true);
			}
		}
		form->parse("GE_HEAD_VAR","GE_HEAD",true);
	}


	//ВЫХОДЫ
	int nst=1;
	QString tdac;
	for(int i=0; i<Count_DAC;i++){

		tx="DAC"+tx1.setNum(i+1);
		tx2="DAC"+tx1.setNum(i+1)+".ERRCUR";

		if(arrs.contains("DEV.OUT")){
			tdac= arrs["DEV.OUT"].p[(int)(i/2)];
			if(tdac!="D" && tdac!="U") continue;
		}
		nst++;

		form->set_var("NDAC","DAC"+tx1.setNum(i+1),false);
		form->parse("DAC_HEAD_VAR","DAC_HEAD",true);
		form->set_var("DAC1",z,false);
		form->set_var("DAC2",z,false);
		form->set_var("DAC3",z,false);
		form->set_var("DAC4",z,false);
		form->set_var("DAC5",z,false);
		form->set_var("DAC6",z,false);
		form->set_var("DAC7",z,false);

		if(arrs.contains(tx)){
			if(arrs[tx].p.at(0)!="0"){
				form->set_var("DAC1",arrs[tx].p.at(0),false);
				tx3=arrs[tx].p.at(0).left(1);
				int in=arrs[tx].p.at(2).toInt(&ok,10)-1;
				form->set_var("DAC2",daparam[tx3].at(in),false);
				form->set_var("DAC3",arrs[tx].p.at(3),false);
				form->set_var("DAC4",arrs[tx].p.at(4),false);
				if(tdac=="D"){
					if(arrs[tx].p.at(1)=="5")	form->set_var("DAC5",stipt.at(0),false); //5
				    else form->set_var("DAC5",stipt.at(1),false);					  //20
				}
				if(tdac=="U"){
					if(arrs[tx].p.at(1)=="5")	form->set_var("DAC5",stipu.at(0),false); //5
					else form->set_var("DAC5",stipu.at(1),false);					  //10
				}
				if(arrs.contains(tx2)){
					if(arrs[tx2].p.at(0)!="0"){
						form->set_var("DAC6",arrs[tx2].p.at(0),false);
						form->set_var("DAC7",arrs[tx2].p.at(1),false);
					}
				}
			}
		}

		form->parse("DAC1_S_VAR","DAC1_S",true);
		form->parse("DAC2_S_VAR","DAC2_S",true);
		form->parse("DAC3_S_VAR","DAC3_S",true);
		form->parse("DAC4_S_VAR","DAC4_S",true);
		form->parse("DAC5_S_VAR","DAC5_S",true);
		form->parse("DAC6_S_VAR","DAC6_S",true);
		form->parse("DAC7_S_VAR","DAC7_S",true);
	}
	form->set_var("DAC_C","COLSPAN="+tx.setNum(nst),false);

	//РЕЛЕ
	nst=1;
	for(int i=0; i<Count_Rele;i++){
		tx="RELE"+tx1.setNum(i+1);

		if(arrs.contains("DEV.OUT")){
			if(arrs["DEV.OUT"].p[Count_DR_Slot-(int)(i/2)-1]!="R") continue;
		}
		nst++;
		//пустышки
		form->set_var("RE1",z,false);
		form->set_var("RE2",z,false);
		form->set_var("RE3",z,false);
		form->set_var("NRE","RE"+tx1.setNum(i+1),false);
		form->parse("RE_HEAD_VAR","RE_HEAD",true);

		if(arrs.contains(tx)){
			//Заполняем
			if(arrs[tx].p[0]!="0"){
				form->set_var("RE1",arrs[tx].p.at(0),false);
				form->set_var("RE2",arrs[tx].p.at(1),false);
				form->set_var("RE3",arrs[tx].p.at(2),false);
			}
			form->parse("RE1_S_VAR","RE1_S",true);
			form->parse("RE2_S_VAR","RE2_S",true);
			form->parse("RE3_S_VAR","RE3_S",true);
		}
	}
	form->set_var("RE_C","COLSPAN="+tx1.setNum(nst),false);
	//485
	tx="RS485";
	if(arrs.contains(tx)){
		form->set_var("RS1",arrs[tx].p.at(0),false);
		form->set_var("RS2",arrs[tx].p.at(1),false);
	}
	tx="VECTOR.ADR";
	if(arrs.contains(tx)){
		form->set_var("RS3",txt.setNum(arrs[tx].p.at(0).toInt(&ok,16)),false);
	}
	//закрываем низ
	form->set_var("END_C","COLSPAN="+tx.setNum(2+nst),false);//
	//КОНЕЦ РЕЛЕ,ВЫХОДЫ,485

	//Тахометры
	int stah=0; //счетчик включенных тахометров
	for(int i=0; i<Count_Tah;i++){
		int s=i+1;
		sn=tx.setNum(s); //число в текст
		//первый тахометр
		//TAH.NAME
		tx="TAH.NAME";
		if(arrs.contains(tx)){
			if(arrs[tx].p.at(i)!="NONAME") form->set_var("T.Name",arrs[tx].p.at(i),false);
			else form->set_var("T.Name",z,false);
		}

		//Уставки
		st="T";
		for(int j=0;j<mvparam[st].count();j++){
			for(int u=0;u<snust.count();u++){
				//T1.F.Level.LR
				tx=st+sn+"."+mvparam[st].at(j).toUpper()+".LEVEL."+snust.at(u).toUpper();
				tx1=mvparam[st].at(j)+"."+snust.at(u);
				//qDebug()<<tx;
				//qDebug()<<tx1;
				form->set_var(tx1,z,false); //сбросили
				if(arrs.contains(tx)){
					if(arrs[tx].p[0]=="1") {
						form->set_var(tx1,arrs[tx].p[1],false);
					}
				}
			}
			//гистерезис A1.Ve.Hyst
			tx=st+sn+"."+mvparam[st].at(j).toUpper()+".HYST";
			tx1=mvparam[st].at(j)+".H";
			form->set_var(tx1,z,false); //сбросили
			if(arrs.contains(tx)){
				form->set_var(tx1,arrs[tx].p[0],false);
			}
		}

		if (s == 1) {
			//TAH1REP
			tx = "TAH1REP";
			if (arrs.contains(tx)) {
				if (arrs[tx].p[0] != "0") {//повторитель - не одноканальный
					form->set_var("N","T1",false);
					form->set_var("TR",s1tah.at(arrs[tx].p.at(0).toInt(&ok,10)-1),false);
					form->parse("T_BODY_VAR", "T_BODY",true);
					continue;
				}else{
					form->set_var("TR",s1tah.at(0),false);
				}
			}
		}

		//TAHhx.CTZ
		tx="TAH"+sn+".CTZ";
		if(arrs.contains(tx)){
			if(arrs[tx].p[1]!="0"){
				form->set_var("N","T"+sn,false);
				form->set_var("TR",s1tah.at(0),false);
				int in = arrs[tx].p.at(1).toInt(&ok,10);
				if(in<3){
						form->set_var("TT",sitah[in],false);
				}else{
						form->set_var("TT","M"+tx1.setNum(in-3+1),false);
				}

				form->set_var("TC",arrs[tx].p[0],false);
				form->set_var("TN",arrs[tx].p[2],false);
				stah++;
			} else 	continue;
		} else continue;

		//TAHhx.Factor
		tx="TAH"+sn+".FACTOR";
		if(arrs.contains(tx)){
			form->set_var("TK",arrs[tx].p[0],false);
		}

		//Tahx.Default
		tx="TAH"+sn+".DEFAULT";
		if(arrs.contains(tx)){
			form->set_var("TD",arrs[tx].p[0],false);
		}

		form->parse("T_BODY_VAR", "T_BODY",true);
	}

	if(stah>0) form->parse("T_HEAD_VAR", "T_HEAD",false);
	//Конец работы с тахометрами



	//Работа с ADC
	for(int i=0; i<Count_ADC;i++){
		ia=1;
		ic=1;
		int s=i+1;
		sn=tx.setNum(s); //число в текст
		//"ADC.OUT"
		tx="ADC.OUT";
		if(arrs.contains(tx)){
			st=arrs[tx].p.at(i);	//имя <<"T"<<"A"<<"C"<<"D"<<"E"<<"F"<<"M";
			if(snkan.indexOf(st)<1) continue; // не назначен - на выход -  не разбираем далее
			form->set_var("N",st+sn,false);
		}
		//"ADC.NAME"
		tx="ADC.NAME";
		if(arrs.contains(tx)) {
			if(arrs[tx].p[i]!="NONAME") form->set_var("ADC.Name",arrs[tx].p.at(i),false);
			else form->set_var("ADC.Name",z,false);
		}
		//AV.FFILTER
		tx="AV.FILTER";
		if(arrs.contains(tx)) form->set_var("AV.Filter",avfilter.at(arrs[tx].p.at(i).toInt(&ok,10)),false);
		else{
			tx1="AV.FILTERHI";
			tx2="AV.FILTERLO";
			if(arrs.contains(tx1) && arrs.contains(tx2)){
				float zn1=arrs[tx1].p.at(i).toFloat();
				float zn2=arrs[tx2].p.at(i).toFloat();
				QString st1,st2;
				st1.setNum(zn1,'g'); st1.replace(".",",");
				st2.setNum(zn2,'g'); st2.replace(".",",");
				tx3=avHIfilter.at(avHIfilter.indexOf(st1))+".."+avLOfilter.at(avLOfilter.indexOf(st2));
				form->set_var("AV.Filter",tx3,false);
			}
		}

		//RV.FFILTER
		tx="RV.FILTER";
		if(arrs.contains(tx)) form->set_var("RV.Filter",rvfilter.at(arrs[tx].p.at(i).toInt(&ok,10)),false);
		//VIB1.TAH
		tx="VIB"+sn+".TAH";
		if(arrs.contains(tx)){
			if(arrs[tx].p[0]!="0") form->set_var("VIBx.T","T"+arrs[tx].p.at(0),false);
			else form->set_var("VIBx.T",z,false);
			form->set_var("VIBx.TN",arrs[tx].p.at(1),false);
		}
		//VIB1.TAH2
		tx="VIB"+sn+".TAH2";
		if(arrs.contains(tx)){
			if(arrs[tx].p[0]!="0") form->set_var("VIBx.T2","T"+arrs[tx].p.at(0),false);
			else form->set_var("VIBx.T2",z,false);
			form->set_var("VIBx.TN2",arrs[tx].p.at(1),false);
		}
		//VIB1.TAH3
		tx="VIB"+sn+".TAH3";
		if(arrs.contains(tx)){
			if(arrs[tx].p[0]!="0") form->set_var("VIBx.T3","T"+arrs[tx].p.at(0),false);
			else form->set_var("VIBx.T3",z,false);
			form->set_var("VIBx.TN3",arrs[tx].p.at(1),false);
		}

		//Cx.ROTZONES
		tx="C"+sn+".ROTZONES";
		if(arrs.contains(tx)){
			form->set_var("RZ1",arrs[tx].p.at(0),false);
			form->set_var("RZ2",arrs[tx].p.at(1),false);
			form->set_var("RZ3",arrs[tx].p.at(2),false);
		}
		//LUTx
		tx="LUT"+sn;
		if(arrs.contains(tx)){
			if(arrs[tx].p[0]!="0") form->set_var("LUTx",arrs[tx].p.at(0),false);
			else form->set_var("LUTx",z,false);
		}


		//если у канала нет уставок - валим.
		if(mvparam[st].count()==0) continue;

		//что индицируем
		//Adc.Ind

		tx="ADC.IND";
		if(arrs.contains(tx)){
			np=arrs[tx].p.at(i).toInt(&ok,10)-1;
			if(np<0) np=0;

		}


		//Уставки сначала для доп параметров j>=0 - если есть уставки
		for(int j=0;j<mvparam[st].count();j++){

			if(j==np) continue; //не на индикации

			//имя параметра и размерность
			form->set_var("PAR",mvparam[st].at(j),false);
			form->set_var("RAZ",moparam[mvparam[st].at(j)],false);
			form->set_var("IND","",false);

			//гистерезис A1.Ve.Hyst
			tx=st+sn+"."+mvparam[st].at(j).toUpper()+".HYST";
			form->set_var("H",z,false); //сбросили
			if(arrs.contains(tx)){
				form->set_var("H",arrs[tx].p[0],false);
			}else continue; //нет записи о уставке - не включен параметр! Валим до следующего

			for(int u=0;u<snust.count();u++){
				//A1.Ve.Level.LR
				tx=st+sn+"."+mvparam[st].at(j).toUpper()+".LEVEL."+snust.at(u).toUpper();
				tx1=snust.at(u);
				form->set_var(tx1,z,false); //сбросили
				if(arrs.contains(tx)){
					if(arrs[tx].p[0]=="1") {
						form->set_var(tx1,arrs[tx].p[1],false);
					}
				}
			}
			switch (snkan.indexOf(st)){
					case 1: //"A"
							form->parse("A_ADD_VAR", "A_ADD",true);
							ia++;
							break;
					case 2: //"C"
							form->parse("C_ADD_VAR", "C_ADD",true);
							ic++;
							break;
				}
			}

			//Для главного j=np - заполняем в последнюю очередь и всегда
			//имя параметра и размерность
			form->set_var("PAR",mvparam[st].at(np),false);
			form->set_var("RAZ",moparam[mvparam[st].at(np)],false);

			form->set_var("IND","*",false);
			for(int u=0;u<snust.count();u++){
				//A1.Ve.Level.LR
				tx=st+sn+"."+mvparam[st].at(np).toUpper()+".LEVEL."+snust.at(u).toUpper();
				tx1=snust.at(u);
				form->set_var(tx1,z,false); //сбросили
				if(arrs.contains(tx)){
					if(arrs[tx].p[0]=="1") {
						form->set_var(tx1,arrs[tx].p[1],false);
					}
				}
			}
			//гистерезис A1.Ve.Hyst
			tx=st+sn+"."+mvparam[st].at(np).toUpper()+".HYST";
			form->set_var("H",z,false); //сбросили
			if(arrs.contains(tx)){
				form->set_var("H",arrs[tx].p[0],false);
			}

		//который на уставке


		switch (snkan.indexOf(st)){
			case 1: //"A"
					form->set_var("A_ROW","ROWSPAN="+tx1.setNum(ia),false);
					form->parse("A_BODY_VAR", "A_BODY",true);
					form->set_var("A_ADD_VAR","",false);
					break;
			case 2: //"C"
				    form->set_var("C_ROW","ROWSPAN="+tx2.setNum(ic),false);
					form->parse("C_BODY_VAR", "C_BODY",true);
					form->set_var("C_ADD_VAR","",false);
					break;
			case 3: //"D"
					form->parse("D_BODY_VAR", "D_BODY",true);
					break;
		}
	}


	if(arrs["ADC.OUT"].p.contains("A")) form->parse("A_HEAD_VAR", "A_HEAD",false);
	if(arrs["ADC.OUT"].p.contains("C")) form->parse("C_HEAD_VAR", "C_HEAD",false);
	if(arrs["ADC.OUT"].p.contains("D")) form->parse("D_HEAD_VAR", "D_HEAD",false);
	//конец работы с ADC

	//Работа с Virt каналами
	for(int i=0; i<Count_Vir;i++){
		int s=i+1;
		sn=tx.setNum(s); //число в текст
		//"VIR.NAME"
		tx="VIR.NAME";
		if(arrs.contains(tx)){
			if(arrs[tx].p.at(i)!="NONAME") form->set_var("V.Name",arrs[tx].p.at(i),false);
			else form->set_var("V.Name",z,false);
		}
		//VIR.OUT
		tx="VIR.OUT";
		if(arrs.contains(tx)){
			if(arrs[tx].p.at(i)!="0"){
				form->set_var("N",arrs[tx].p.at(i)+sn,false);
			} else continue;
		}

		//Уставки
		if(arrs[tx].p.at(i)=="E") st="E";
		else st="F";

		for(int j=0;j<mvparam[st].count();j++){
			for(int u=0;u<snust.count();u++){
				//E1.Svpp.Level.LR
				tx=st+sn+"."+mvparam[st].at(j).toUpper()+".LEVEL."+snust.at(u).toUpper();
				tx1="X."+snust.at(u);
				//qDebug()<<tx;
				//qDebug()<<tx1;
				form->set_var(tx1,z,false); //сбросили
				if(arrs.contains(tx)){
					if(arrs[tx].p[0]=="1") {
						form->set_var(tx1,arrs[tx].p[1],false);
					}
				}
			}
			//гистерезис E1.Svpp.Level.LR
			tx=st+sn+"."+mvparam[st].at(j).toUpper()+".HYST";
			tx1="X.H";
			form->set_var(tx1,z,false); //сбросили
			if(arrs.contains(tx)){
				form->set_var(tx1,arrs[tx].p[0],false);
			}
		}


		tx=st+sn+".SRC";
		form->set_var("VP",mvparam[st].at(0),false);
		form->set_var("VI",moparam[mvparam[st].at(0)],false);

		if(arrs.contains(tx)){
			form->set_var("VX",arrs[tx].p.at(0),false);
			form->set_var("VY",arrs[tx].p.at(1),false);
		}

		form->parse("V_BODY_VAR", "V_BODY",true);
	}
	if(arrs["VIR.OUT"].p.contains("F") || arrs["VIR.OUT"].p.contains("E")) form->parse("V_HEAD_VAR", "V_HEAD",false);
	//Конец работы с вирт. каналами

	//парсим заполненное
	form->parse("templ", "templ",false);
	//убираем пустышки - присваивая им пустоту
	form->get_undefined("templ","");
	//парсим оставшееся
	form->parse("templ", "templ",false);
	//результат
	form->get_vars("templ");

	//qDebug()<<ptxt;
	return true;
}
