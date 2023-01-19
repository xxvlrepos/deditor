/*
 * mdata.cpp
 *
 *  Created on: 30.10.2012
 *      Author: Usach
 */

#include "mdata.h"



const QVector<QString> mdata::snslot=IniQVector<QString>()<<"TAH1"<<"TAH2"<<"TAH3"<<"ADC1"<<"ADC2"<<"ADC3"<<"ADC4"<<"ADC5"<<"ADC6"<<"VIR1"<<"VIR2"<<"TAHO1"<<"TAHO2"<<"TAHO3";
const QVector<QString> mdata::snkan=IniQVector<QString>()<<"T"<<"A"<<"C"<<"D"<<"I"<<"E"<<"F"<<"M"<<"N";
const QVector<QString> mdata::snust=IniQVector<QString>()<<"LR"<<"3L"<<"2L"<<"1L"<<"1H"<<"2H"<<"3H"<<"HR";
const QVector<QString> mdata::stipt=IniQVector<QString>()<<"0..5mA"<<"4..20mA";
const QVector<QString> mdata::stipu=IniQVector<QString>()<<"0..5V"<<"0..10V";
const QVector<QString> mdata::avfilter=IniQVector<QString>()<<"10..1000"<<"2..1000"<<"20..1000"<<"30..1000"<<"40..150"<<"50..300"<<"30..150"<<"30..400"<<"0,5..250";
const QVector<QString> mdata::avHIfilter=IniQVector<QString>()<<"2"<<"10"<<"20"<<"30"<<"40"<<"50"<<"0,5";
const QVector<QString> mdata::avLOfilter=IniQVector<QString>()<<"1000"<<"700"<<"400"<<"300"<<"250"<<"200"<<"150";
const QVector<QString> mdata::rvfilter=IniQVector<QString>()<<"10..1000"<<"5..500"<<"0,4..1000";
const QVector<QString> mdata::s1tah=IniQVector<QString>()<<tr("exit2")<<tr("exit3");
const QVector<QString> mdata::sitah=IniQVector<QString>()<<tr("disabled")<<tr("input")<<tr("open");
const QVector<QString> mdata::sge=getGE();
const QMap<QString,QStringList> mdata::mvparam = IniQMap<QString,QStringList>()
		<<QPair<QString,QStringList>("T",IniQStringList()<<"F")
		<<QPair<QString,QStringList>("A",IniQStringList()<<"Ve"<<"NV"<<"Spp"<<"Ae"<<"Vp"<<"Vpe"<<"Ap"<<"App")
		<<QPair<QString,QStringList>("C",IniQStringList()<<"Spp"<<"Sp"<<"Z"<<"Se"<<"Sppe")
		<<QPair<QString,QStringList>("D",IniQStringList()<<"Z")
		<<QPair<QString,QStringList>("I",IniQStringList()<<"Z")
		<<QPair<QString,QStringList>("E",IniQStringList()<<"Vexy")
		<<QPair<QString,QStringList>("F",IniQStringList()<<"Sp2xy")
		<<QPair<QString,QStringList>("N",IniQStringList()<<"TEMP");
const QMap<QString,QStringList> mdata::daparam = IniQMap<QString,QStringList>()
		<<QPair<QString,QStringList>("T",IniQStringList()<<"F"<<"Mraw")
		<<QPair<QString,QStringList>("A",IniQStringList()<<"Ve"<<"A"<<"V"<<"S"<<"Vp"<<"Ae"<<"Vpe"<<"NV"<<"Spp"<<"Araw"<<"Ap"<<"App")
		<<QPair<QString,QStringList>("C",IniQStringList()<<"Z"<<"Sp"<<"Spp"<<"S"<<"Sb"<<"Mb"<<"Sraw"<<"Se"<<"Sppe")
		<<QPair<QString,QStringList>("D",IniQStringList()<<"Z"<<"Sraw")
		<<QPair<QString,QStringList>("M",IniQStringList()<<"RAW"<<"Z")
		<<QPair<QString,QStringList>("E",IniQStringList()<<"Vexy")
		<<QPair<QString,QStringList>("F",IniQStringList()<<"Sp2xy")
		<<QPair<QString,QStringList>("N",IniQStringList()<<"TEMP");

const QMap<QString,QString> mdata::moparam = IniQMap<QString,QString>()
		<<QPair<QString,QString>("Ve",tr("mm/c"))
		<<QPair<QString,QString>("NV",tr("mm/c"))
		<<QPair<QString,QString>("Spp",tr("mkm"))
		<<QPair<QString,QString>("Sp",tr("mkm"))
		<<QPair<QString,QString>("Se",tr("mkm"))
		<<QPair<QString,QString>("Z",tr("mkm"))
		<<QPair<QString,QString>("F",tr("rotation/min"))
		<<QPair<QString,QString>("Vexy",tr("mm/c"))
		<<QPair<QString,QString>("Sp2xy",tr("mkm"))
		<<QPair<QString,QString>("A",tr("m/c^2"))
		<<QPair<QString,QString>("Araw",tr("m/c^2"))
		<<QPair<QString,QString>("Ae",tr("m/c^2"))
		<<QPair<QString,QString>("Ap",tr("m/c^2"))
		<<QPair<QString,QString>("App",tr("m/c^2"))
		<<QPair<QString,QString>("V",tr("mm/c"))
		<<QPair<QString,QString>("S",tr("mkm"))
		<<QPair<QString,QString>("Sb",tr("mkm"))
		<<QPair<QString,QString>("Mb",tr("bin"))
		<<QPair<QString,QString>("Mraw",tr("bin"))
		<<QPair<QString,QString>("Sraw",tr("mkm"))
		<<QPair<QString,QString>("Vp",tr("mm/c"))
		<<QPair<QString,QString>("Vpe",tr("mm/c"))
		<<QPair<QString,QString>("S1",tr("mkm"))
		<<QPair<QString,QString>("S2",tr("mkm"))
		<<QPair<QString,QString>("RAW",tr("---"))
		<<QPair<QString,QString>("TEMP",tr("C"));



