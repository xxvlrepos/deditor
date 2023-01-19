/*
 * xmlschema.cpp
 *
 *  Created on: 22.05.2012
 *      Author: Usach
 */

#include "xmlschema.h"
#include <QtCore>
#include <QXmlSchema>
#include <QXmlSchemaValidator>
//#include <QtGui/QMessageBox>

xmlschema::xmlschema(const QString& xsdName,QDomDocument* xml) {

	QXmlSchema sch;
	Valid=false;
	QFile filexsd(xsdName);
	if(!filexsd.open(QIODevice::ReadOnly | QIODevice::Text)){
		     qDebug() <<"Not open file:"+xsdName;
		     return;
	}else{
			if(!xsddoc.setContent(&filexsd)){
					qDebug() <<"Error parse file:"+xsdName;
					filexsd.close();
					return;
			}else{
					filexsd.close();
					sch.load(xsddoc.toByteArray());
					if(!sch.isValid()){
						 qDebug() <<"Schema invalid:"+xsdName;
						 return;
					}
					Valid=true;
			}
	}

	QDomElement xsdE;
	xsdE=xsddoc.documentElement();
	//получаем namesp схемы
	QStringList list=xsdE.nodeName().split(":");
	if(list.count()>1){
		namesp=list.at(0);
		namespp=namesp+":";

	}
	rootxsd=xsdE;

	//заполняем словари автономных типов
		QVector<QDomNode> vec;
		vec=SearchChild(rootxsd.toElement(),namespp+"element");
		if(vec.size()>0) FirstElName=vec[0].toElement().attribute("name");
		for(int i=0;i<vec.count();i++) elementMap[vec.at(i).attributes().namedItem("name").nodeValue()]=vec.at(i);
		vec=SearchChild(rootxsd.toElement(),namespp+"complexType");
		for(int i=0;i<vec.count();i++) complexTypeMap[vec.at(i).attributes().namedItem("name").nodeValue()]=vec.at(i);
		vec=SearchChild(rootxsd.toElement(),namespp+"simpleType");
		for(int i=0;i<vec.count();i++) simpleTypeMap[vec.at(i).attributes().namedItem("name").nodeValue()]=vec.at(i);
		vec=SearchChild(rootxsd.toElement(),namespp+"attribute");
		for(int i=0;i<vec.count();i++) attributeMap[vec.at(i).attributes().namedItem("name").nodeValue()]=vec.at(i);

		vec=SearchChild(rootxsd.toElement(),namespp+"attributeGroup");
		for(int i=0;i<vec.count();i++) attributeGroupMap[vec.at(i).attributes().namedItem("name").nodeValue()]=vec.at(i);
		vec=SearchChild(rootxsd.toElement(),namespp+"group");
		for(int i=0;i<vec.count();i++) groupMap[vec.at(i).attributes().namedItem("name").nodeValue()]=vec.at(i);


	xmldoc=xml;
    SxsdName=xsdName.section('/',-1);


	ValidDoc=true;
	if(xml->isNull()){
		xmldoc->setContent(defaultXML());
		//qDebug()<<txt;
	}else{
		QXmlSchemaValidator validator(sch);
		if (!validator.validate(xmldoc->toByteArray())){
			    xmldoc->setContent(defaultXML());
				qDebug() <<"XML document invalid! Create default.";
				//QMessageBox::warning(this,"", "XML document invalid!.Creade default.");
				ValidDoc=false;
		 }
	}

	QDomElement xmlE;
	xmlE=xmldoc->documentElement();
	rootxml=xmlE;
	rootHmlToXsd=getNode(rootxml.toElement());
}

xmlschema::~xmlschema() {
	// TODO Auto-generated destructor stub
}

QString xmlschema::defaultXML(){
	QString txt;
	txt+="<?xml version='1.0' encoding='UTF-8'?>\n";
	txt+="<"+FirstElName+" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:noNamespaceSchemaLocation=\""+SxsdName+"\">\n";
	txt+="</"+FirstElName+">\n";
	return txt;
}

/*
 * получить описание элемента в схеме
 */
QDomNode xmlschema::getNode(QDomElement el){
		QList<QDomNode> listNode;
		QDomNode   xsdnode;
		QDomNode   xsdType;
		if(el.isNull()) return QDomNode();

		//получаем список всех родителей до рута
		listNode<<el;

		if(el!=rootxml){
			QDomNode cur=el.parentNode();
			while(cur!=rootxml){
				listNode<<cur;
				cur=cur.parentNode();
			}
		}
		//спускаясь по родителям обратно находим элемент
		if(elementMap.contains(rootxml.nodeName())) xsdnode=elementMap[rootxml.nodeName()];

		QDomNodeList elemList;
		for(int i=listNode.size()-1;i>=0;i--){//идем по узлам сверху-вниз
			QDomNode xsdType=getTypeXLS(xsdnode.toElement());
			if(xsdType.isNull()) return QDomNode(); //нету
			//список описаний элементов -ищем в нем свой
			elemList=xsdType.toElement().elementsByTagName(namespp+"element");
			for(int k=0;k<elemList.count();k++){
				if(getNameXLS(elemList.at(k))==listNode.at(i).nodeName()){
					xsdnode=elemList.at(k);
					break;
				}
			}
		}
		return xsdnode;
}

/*
 * получить описание типа элемента в схеме
 */
QDomNode xmlschema::getType(QDomElement el){
	return getTypeXLS(getNode(el).toElement());
}

/*
 * получить описание типа элемента XLS
 */
QDomNode xmlschema::getTypeXLS(QDomElement xsdnode){

    QString Type;
    if(xsdnode.isNull()) return QDomNode();
    QDomElement cur=xsdnode;

    if(xsdnode.attributes().contains("ref")){
    	QString name=xsdnode.attributes().namedItem("ref").nodeValue();
		if(elementMap.contains(name)) cur=elementMap[name].toElement();
    }

	Type=cur.attributes().namedItem("type").nodeValue();
	if(Type!=""){
		if(complexTypeMap.contains(Type)) return complexTypeMap[Type];
		if(simpleTypeMap.contains(Type))  return complexTypeMap[Type];
		return QDomNode();
	}else{
		//if(cur.nodeName()==namespp+"group")           return cur;
		//if(cur.nodeName()==namespp+"attributeGroup")  return cur;
		if(cur.firstChild().nodeName()==namespp+"complexType") return cur.firstChild();
		if(cur.firstChild().nodeName()==namespp+"simpleType")  return cur.firstChild();
		return QDomNode();
	}
	return QDomNode();
}

/*
 * получить виджет для редактирования атрибута в зависимости от его типа по схеме
 */

QWidget* xmlschema::getAttributeWidget(QDomAttr* atr,QWidget *parent){

	QDomNode   xsdnode,unode;
	QString    tatt="";
	QWidget*    rez=NULL;
	bool ok;

	//qDebug()<<atr->parentNode().toElement().nodeName();
	QVector<QDomNode> attrList=getAttributes(atr->parentNode().toElement());
	for(int i=0;i<attrList.count();i++){
		if(attrList.at(i).attributes().namedItem("name").nodeValue()==atr->name()){
			xsdnode=attrList.at(i);
			break;
		}
	}
	if(xsdnode.isNull()) return new QLineEdit(parent);

	//Ищем тип
	//атрибут по ссылке
	if(xsdnode.attributes().contains("ref")){
		QString name=xsdnode.attributes().namedItem("ref").nodeValue();
		if(attributeMap.contains(name)){
			xsdnode=attributeMap[name];
			xsdnode=xsdnode.firstChild(); //<xsd:simpleType>
			xsdnode=xsdnode.firstChild();//xsd:restriction
			tatt=xsdnode.attributes().namedItem("base").nodeValue();
		}
	}

	//если значение фиксированное - нельзя редактировать
	if(xsdnode.attributes().contains("fixed")) return new QWidget(parent);

	//стандартный тип
	if(tatt=="" && xsdnode.attributes().contains("type")){
		//стандарт
		tatt=xsdnode.attributes().namedItem("type").nodeValue();
		if(simpleTypeMap.contains(tatt)){ //не стандарт
			xsdnode=simpleTypeMap[tatt];
			xsdnode=xsdnode.firstChild();//xsd:restriction
			tatt=xsdnode.attributes().namedItem("base").nodeValue();
		}
	}

	if(tatt==""){
		xsdnode=xsdnode.firstChild(); //<xsd:simpleType>
		xsdnode=xsdnode.firstChild();//xsd:restriction
		tatt=xsdnode.attributes().namedItem("base").nodeValue();
	}



	//Устанавливаем параметры элемента
	long long int min=0,max=0,cur=0;
	QVector<QDomNode> propList,propListEnum,propListPattern;

	// проверка на union
	if(tatt==""){
		if(xsdnode.nodeName()==namespp+"union"){
			QDomNodeList UList=xsdnode.childNodes();
			for(int i=0;i<UList.count();i++){
				unode=UList.at(i);        //<xsd:simpleType>
				unode=unode.firstChild();//xsd:restriction
				if(i==0) xsdnode=unode;
				tatt=unode.attributes().namedItem("base").nodeValue();
				propListEnum=SearchChild(unode.toElement(),namespp+"enumeration");
				propListPattern=SearchChild(unode.toElement(),namespp+"pattern");
			}
		}
	}

	//не нашли
	if(tatt=="") return new QLineEdit(parent);


	if(propListEnum.isEmpty())    propListEnum=SearchChild(xsdnode.toElement(),namespp+"enumeration");
	if(propListPattern.isEmpty()) propListPattern=SearchChild(xsdnode.toElement(),namespp+"pattern");

    //если есть фикс список - тип значения уже не важен
	if(!propListEnum.isEmpty()){
		QComboBox* cb = new QComboBox(parent);
		for(int i=0;i<propListEnum.count();i++){
				cb->addItem(propListEnum.at(i).attributes().namedItem("value").nodeValue());
		}
		if(!propListPattern.isEmpty()){
			cb->setEditable(true);
			QString pat=propListPattern.at(0).attributes().namedItem("value").nodeValue();
			//qDebug()<<pat;
			QRegExp rx(pat);
			QValidator *validator = new QRegExpValidator(rx,parent);
			cb->setValidator(validator);
		}
		rez=cb;
	}
	else if(tatt==namespp+"string"){
		QLineEdit* le=new QLineEdit(parent);
		propList=SearchChild(xsdnode.toElement(),namespp+"length");
		if(propList.count()>0){
			le->setMaxLength(propList.at(0).attributes().namedItem("value").nodeValue().toInt(&ok,10));
		}
		propList=SearchChild(xsdnode.toElement(),namespp+"maxLength");
		if(propList.count()>0){
			le->setMaxLength(propList.at(0).attributes().namedItem("value").nodeValue().toInt(&ok,10));
		}

		if(!propListPattern.isEmpty()){
			QString pat=propListPattern.at(0).attributes().namedItem("value").nodeValue();
			//qDebug()<<pat;
			QRegExp rx(pat);
			QValidator *validator = new QRegExpValidator(rx,parent);
			le->setValidator(validator);
		}
		rez=le;

	}
	else if(tatt==namespp+"int" || tatt==namespp+"short" || tatt==namespp+"long" || tatt==namespp+"byte"){
		QSpinBox* sb = new QSpinBox(parent);

		if(tatt==namespp+"byte") 							  {min=-1*0x7f; max=0x7F;}
		else if(tatt==namespp+"short") 						  {min=-1*0x7FFF; max=0x7FFF;}
		else if(tatt==namespp+"int" || tatt==namespp+"long")  {min=-1*0x7FFFFFFF; max=0x7FFFFFFF;}

		corectIntDiap(min,max,xsdnode);
		sb->setMinimum(min);
		sb->setMaximum(max);
		rez=sb;
	}
	else if(tatt==namespp+"unsignedInt" || tatt==namespp+"unsignedShort" || tatt==namespp+"unsignedLong" || tatt==namespp+"unsignedByte"){
			QSpinBox* usb = new QSpinBox(parent);
			min=0;
			if(tatt==namespp+"unsignedByte") 								     max=0xFF;
			else if(tatt==namespp+"unsignedShort") 								 max=0xFFFF;
			else if(tatt==namespp+"unsignedInt" || tatt==namespp+"unsignedLong") max=0xFFFFFFFF;

			corectIntDiap(min,max,xsdnode);
			usb->setMinimum(min);
			usb->setMaximum(max);
			rez=usb;
	}
	else if(tatt==namespp+"hexBinary"){
			HexSpinBox* hsb = new HexSpinBox(parent);
			propList=SearchChild(xsdnode.toElement(),namespp+"minInclusive");
			if(propList.count()>0){
				hsb->setMinimum(propList.at(0).attributes().namedItem("value").nodeValue().toInt(&ok,10));
			}
			propList=SearchChild(xsdnode.toElement(),namespp+"maxInclusive");
			if(propList.count()>0){
				hsb->setMaximum(propList.at(0).attributes().namedItem("value").nodeValue().toInt(&ok,10));
			}
			rez=hsb;
	}
	else if(tatt==namespp+"float" || tatt==namespp+"decimal"|| tatt==namespp+"double"){
		QDoubleSpinBox* dsb=new QDoubleSpinBox(parent);
		propList=SearchChild(xsdnode.toElement(),namespp+"minInclusive");
		if(propList.count()>0){
			dsb->setMinimum(propList.at(0).attributes().namedItem("value").nodeValue().toFloat());
		}
		propList=SearchChild(xsdnode.toElement(),namespp+"maxInclusive");
		if(propList.count()>0){
			dsb->setMaximum(propList.at(0).attributes().namedItem("value").nodeValue().toFloat());
		}
		propList=SearchChild(xsdnode.toElement(),namespp+"fractionDigits");
		if(propList.count()>0){
			dsb->setDecimals(propList.at(0).attributes().namedItem("value").nodeValue().toInt(&ok,10));
		}

		rez = dsb;
	}
	else if(tatt==namespp+"boolean"){
		QComboBox* cbb = new QComboBox(parent);
		cbb->addItem("0");
		cbb->addItem("1");
		rez=cbb;

	}
	else if(tatt==namespp+"date"){
		   QDateEdit* dat=new QDateEdit(parent);
		   QLocale loc(QLocale::English, QLocale::UnitedStates);
		   QString str;
			propList=SearchChild(xsdnode.toElement(),namespp+"minInclusive");
			if(propList.count()>0){
				str=propList.at(0).attributes().namedItem("value").nodeValue();
				dat->setMinimumDate(loc.toDateTime(str,"yyyy-MM-dd").date());
			}
			propList=SearchChild(xsdnode.toElement(),namespp+"maxInclusive");
			if(propList.count()>0){
				str=propList.at(0).attributes().namedItem("value").nodeValue();
				dat->setMaximumDate(loc.toDateTime(str,"yyyy-MM-dd").date());
			}
			rez = dat;
	}

	if(rez==NULL) rez= new QLineEdit(parent);

	return rez;
}


void xmlschema::corectIntDiap(long long int &min,long long int &max,QDomNode xsdnode){
	long long int cur;
	QVector<QDomNode> propList;
	bool ok;

	cur=min;
	propList=SearchChild(xsdnode.toElement(),namespp+"minInclusive");
	if(propList.count()>0){
		cur=propList.at(0).attributes().namedItem("value").nodeValue().toInt(&ok,10);
	}else{
		propList=SearchChild(xsdnode.toElement(),namespp+"minExclusive");
		if(propList.count()>0){
			cur=propList.at(0).attributes().namedItem("value").nodeValue().toInt(&ok,10)+1;
		}
	}
	if(cur>min) min=cur;

	cur=max;
	propList=SearchChild(xsdnode.toElement(),namespp+"maxInclusive");
	if(propList.count()>0){
		cur=propList.at(0).attributes().namedItem("value").nodeValue().toInt(&ok,10);
	}else{
		propList=SearchChild(xsdnode.toElement(),namespp+"maxExclusive");
		if(propList.count()>0){
			cur=propList.at(0).attributes().namedItem("value").nodeValue().toInt(&ok,10)-1;
		}
	}
	if(cur<max) max=cur;
}

/*
 * получить список описаний детей елемента xml по схеме
 */
QVector<QDomNode> xmlschema::getChild(QDomElement el){

	QDomNode   xsdnode;
	xsdnode=getType(el);
	QVector<QDomNode> rez = getElementXLS(xsdnode.toElement());
	return rez;
}

/*
 * получить список имен детей елемента xml по схеме
 */
QStringList xmlschema::getChildName(QDomElement el){
	QStringList rez;
	QVector<QDomNode> xlsc=getChild(el);
	for(int i=0;i<xlsc.count();i++)	rez<<getNameXLS(xlsc.at(i));
	return rez;
}

/*
 * Ищем рекурсивно все описания элементов внутри типа, отношений или группы
 */
QVector<QDomNode> xmlschema::getElementXLS(const QDomElement el){
	    QVector<QDomNode> rez;
	    QDomNodeList cild=el.childNodes();	//список детей
	    for(int i=0;i<cild.count();i++){
	    	QString name=cild.at(i).nodeName();
	    	if(name==namespp+"sequence" || name==namespp+"all" || name==namespp+"choice"){
	    		rez<<getElementXLS(cild.at(i).toElement());
	    	}else
	    	if(name==namespp+"element"){
	   			rez<<cild.at(i);
	    	}else
	    	if(name==namespp+"group"){
	    		QString grname=cild.at(i).attributes().namedItem("ref").nodeValue();
	    		if(groupMap.contains(grname)){
	    			rez<<getElementXLS(groupMap[grname].toElement());
	    		}
	    	}

	    }
	    return rez;
}

/*
 * получить список описаний детей элемента  по схеме, которые еще можно добавить
 */
QVector<QDomNode> xmlschema::getAddChild(QDomElement el){
	QVector<QDomNode> rez;
    QDomNodeList cild=el.childNodes();	   //дети по факту
	QVector<QDomNode> xlsc=getChild(el);   //описания детей
	int acou=el.childNodes().count();//всего потомков уже есть

	for(int i=0;i<xlsc.count();i++){
		QString  name=getNameXLS(xlsc.at(i));
		int cou=SearchChild(el,name).count();                        //количество данного елемента по факту
		switch(getXLSTypeRatio(xlsc.at(i))){
			case sequence:
			case all:
				if(getMaxOccursXLS(xlsc.at(i))>0){//есть ограничение сверху
					if(cou<getMaxOccursXLS(xlsc.at(i))) rez.append(xlsc.at(i));
				}else rez.append(xlsc.at(i));
				break;
			case choice://если пусто - то любой, иначе один в зависимости от количества допустимых

				if(acou==0) rez.append(xlsc.at(i));//элементов нет - добавляем все
				else{//какой-то есть - смотрим макс количество по родителю
					int max=getMaxOccursXLS(xlsc.at(i));
					if(max==0) {rez.append(xlsc.at(i)); break;}//не ограничено
					if(max>1){
						if(max>acou) rez.append(xlsc.at(i));
					}
				}
				break;
		}

	}
	return rez;
}

/*
 * получить список имен детей элемента  по схеме, которые еще можно добавить
 */
QStringList xmlschema::getAddChildName(QDomElement el){
	QVector<QDomNode> xlsc;
	QStringList rez;
	xlsc=getAddChild(el);

	for(int i=0;i<xlsc.count();i++) rez<<getNameXLS(xlsc.at(i));//его имя
	return rez;
}

/*
 * получить список описаний других детей по схеме, на которые можно заменить текущий
 * актуально для отношений choice и all
 */
QVector<QDomNode> xmlschema::getReplaice(QDomElement ci){
	QVector<QDomNode> rez;

	if(ci==rootxml.toElement()) return rez;
	QDomElement el=ci.parentNode().toElement();//папа элемента


	NodeRatio Ratio=getXLSTypeRatio(getNode(ci));
	if(Ratio==sequence) return rez; //для данного случая нельзя заменять, так как порядок добавления строгий
	//по идее можно - если соседи

	//можно-ли удалить?
	int nmin;//минимум
	int cou=SearchChild(el,ci.tagName()).count(); //по факту уже
	if(Ratio==choice) nmin=0;//всегда можно удалить
	else              nmin=getMinOccursXLS(getNode(ci));//минимум
	if(cou<=nmin) return rez; //нельзя удалять исходный

    //на что можно заменить?
	QVector<QDomNode> xlsc=getChild(el);        //описания детей
	for(int i=0;i<xlsc.count();i++){
		QString name=getNameXLS(xlsc.at(i));//его имя
		if(name==ci.tagName()) continue; 		//на самого себя не меняем
		int cou=SearchChild(el,name).count();   //по факту
		switch(Ratio){
			case all://любой из доступных, чей лимит не исчерпан
				if(getMaxOccursXLS(xlsc.at(i))>0){//есть ограничение сверху
					if(cou<getMaxOccursXLS(xlsc.at(i))) rez.append(xlsc.at(i));
				}else rez.append(xlsc.at(i));
				break;
			case choice://любой из доступных
				rez.append(xlsc.at(i));// добавляем все
				break;
			case sequence: break;//чисто формально
		}
	}
	return rez;
}

/*
 * получить список имен детей элемента  по схеме, можно заменить
 */
QStringList xmlschema::getReplaiceName(QDomElement el){
	QVector<QDomNode> xlsc;
	QStringList rez;
	xlsc=getReplaice(el);
	for(int i=0;i<xlsc.count();i++) rez<<getNameXLS(xlsc.at(i));//его имя
	return rez;
}

/*
 * получить список атрибутов елемента xml по схеме
 */
QVector<QDomNode> xmlschema::getAttributes(QDomElement el, getTypeAttr type){
	return getAttributesXLS(getType(el), type);
}

/*
 * Получить все атрибуты из найденного описания элемента в схеме
 */
QVector<QDomNode> xmlschema::getAttributesXLS(QDomNode xsdnode, getTypeAttr type){
	QVector<QDomNode> rez;
	//QDomNode   attr;
	QDomNodeList child;
	typeUseAttr  ctype;

	child=xsdnode.childNodes();
	for(int i=0;i<child.count();i++){
		//просто атрибуты и ссылки
		if(child.at(i).nodeName()==namespp+"attribute"){
			if(child.at(i).toElement().hasAttribute("ref")){
				QString name=child.at(i).attributes().namedItem("ref").nodeValue();
				if(attributeMap.contains(name)){
						ctype=getTypeUseAttr(child.at(i));
						if((getTypeAttr)ctype==type || type==getAll) rez<<attributeMap[name];
						continue;
				}
			}
			//типы атрибутов
			ctype=getTypeUseAttr(child.at(i));
			if((getTypeAttr)ctype==type || type==getAll) rez<<child.at(i);
			continue;
		}

		//группы атрибутов и ссылки
		if(child.at(i).nodeName()==namespp+"attributeGroup"){
			if(child.at(i).toElement().hasAttribute("ref")){
				QString name=child.at(i).attributes().namedItem("ref").nodeValue();
				if(attributeGroupMap.contains(name)){
					rez<<getAttributesXLS(attributeGroupMap[name].toElement(),type);
				}
			}
			//else{  //небывает
			//	rez<<getAttributes(child.at(i).toElement(),type);
			//}
		}

	}
	return rez;
}


/*
 * получить список атрибутов елемента xml по схеме
 */
QStringList xmlschema::getAttributesName(QDomElement el, getTypeAttr type){
	    QVector<QDomNode> xlsc;
		QStringList rez;
		xlsc=getAttributes(el,type);

		for(int i=0;i<xlsc.count();i++) rez<<getNameXLS(xlsc.at(i));//его имя
		return rez;
}

/*
 * отношения детей внутри типа элемента (сверху)
 */
NodeRatio xmlschema::getXLSTypeRatioCild(QDomNode el){

	NodeRatio Ratio=all;
	QDomNode cild=el.firstChild();
	QString np=cild.toElement().tagName();
	if(np==namespp+"sequence") 			Ratio=sequence;
	else if(np==namespp+"all") 			Ratio=all;
	else if(np==namespp+"choice") 		Ratio=choice;

	return Ratio;
}

/*
 * отношения детей внутри группы (снизу)
 */
NodeRatio xmlschema::getXLSTypeRatio(QDomNode el){

	NodeRatio Ratio=all;
	if(el!=rootxsd){
		QDomNode parent=el.parentNode();
		QString np=parent.toElement().tagName();
		if(np==namespp+"sequence") 		Ratio=sequence;
		else if(np==namespp+"all") 		Ratio=all;
		else if(np==namespp+"choice") 	Ratio=choice;
	}
	return Ratio;
}

typeUseAttr xmlschema::getTypeUseAttr(QDomNode el){
	typeUseAttr type=useOtional;
	if(el.toElement().hasAttribute("use")) {
		QString tx=el.attributes().namedItem("use").nodeValue();
		if(tx=="optional") type=useOtional;
		else if(tx=="required") type=useRequired;
		else if(tx=="prohibited") type=useProhibited;
	}else{//нарушение - при наличии дефолта бе указания use
		if(el.toElement().hasAttribute("default")) type=useRequired;
	}
	return type;
}

/*
 * получить значение атрибута по умолчанию
 */
QString  xmlschema::getAttributeXLSDefault(QDomNode atr){

	QList<QDomNode> listNode;
	QDomNode   xsdnode;
	QString    tatt;
	QString    rez="";
	//bool ok;



	if(atr.attributes().contains("default")){
		return atr.attributes().namedItem("default").nodeValue();
	}
	if(atr.attributes().contains("fixed")){
		return atr.attributes().namedItem("fixed").nodeValue();
	}

	tatt=atr.attributes().namedItem("type").nodeValue();
	if(tatt!=""){
		if(tatt!=namespp+"string") rez="0";
		return rez;
	}

    //идем внутрь описания атрибута
	xsdnode=atr.firstChild(); //<xsd:simpleType>
	xsdnode=xsdnode.firstChild();//xsd:restriction
	tatt=xsdnode.attributes().namedItem("base").nodeValue();


	//Устанавливаем параметры элемента
	QVector<QDomNode> propList;
	propList=SearchChild(xsdnode.toElement(),namespp+"enumeration");
	if(propList.count()>0){
			rez=propList.at(0).attributes().namedItem("value").nodeValue();
			return rez;
	}

	propList=SearchChild(xsdnode.toElement(),namespp+"minInclusive");
	if(propList.count()>0){
			rez=propList.at(0).attributes().namedItem("value").nodeValue();
			return rez;
	}

	return rez;
}

/*
 * получить min кратность элемента схемы
 */
int  xmlschema::getMinOccursXLS(QDomNode el){
	bool ok;
	int rez=1;

	if(el.attributes().contains("minOccurs")){
		rez=el.attributes().namedItem("minOccurs").nodeValue().toInt(&ok,10);
	}
	return rez;
}

/*
 * получить max кратность элемента схемы
 * 0-не ограничено
 */
int  xmlschema::getMaxOccursXLS(QDomNode el){
	bool ok;
	int rez=1;
	int par=1;
	if(getXLSTypeRatio(el)==choice) par=getMaxOccursXLS(el.parentNode());

	if(el.attributes().contains("maxOccurs")){
		if(el.attributes().namedItem("maxOccurs").nodeValue()=="unbounded"){
			return 0;
		}
		rez=par*el.attributes().namedItem("maxOccurs").nodeValue().toInt(&ok,10);
	}else{
		rez=par;
	}

	return rez;
}

/*
 * список описаний элементов для вставки до указанного
 */
QVector<QDomNode>  xmlschema::getInsBeforeElements(QDomElement de){

	QVector<QDomNode> vadd;

	if(de==rootxml.toElement()) return vadd;
	QDomElement pe=de.parentNode().toElement();
	QVector<QDomNode> alladd= getAddChild(pe);//список, чего можно добавить
	QVector<QDomNode> xlsc=getChild(pe); //список описаний детей
	if(xlsc.size()==0) return alladd;
	if(getXLSTypeRatio(xlsc.at(0))==sequence){
		//предшественник
		QDomNode pre;
		if(!de.previousSiblingElement().isNull()) pre=de.previousSiblingElement();

		//ищем в списке данный
		int i=0;
		for(i=0;i<xlsc.count();i++){
			if(getNameXLS(xlsc.at(i))==de.nodeName()){
				break;
			}
		}

		//ищем в списке предшественника
		int j;
		if(pre.isNull()){ j=0;}
		else{
			for(j=i;j>=0;j--){
				if(getNameXLS(xlsc.at(j))==pre.nodeName()){
					break;
				}
			}
		}

		//идем вверх по списку, проверяя наличие в списке допустимых
		for(int k=i;k>=0;k--){
			if(k>=j){//пока не дошли до предшественника - добавляем все
				if(alladd.contains(xlsc.at(k))) vadd.prepend(xlsc.at(k));
			}else break;

		}
	}else{
		vadd=alladd;
	}

	return vadd;
}

/*
 * получить список имен детей элемента  по схеме, можно вставить до
 */
QStringList xmlschema::getInsBeforeElementsName(QDomElement el){
	QVector<QDomNode> xlsc;
	QStringList rez;
	xlsc=getInsBeforeElements(el);

	for(int i=0;i<xlsc.count();i++) rez<<getNameXLS(xlsc.at(i));
	return rez;
}

/*
 * список описаний элементов для вставки после указанного
 */
QVector<QDomNode>  xmlschema::getInsAfterElements(QDomElement de){

	QVector<QDomNode> vadd;

		if(de==rootxml.toElement()) return vadd;
		QDomElement pe=de.parentNode().toElement();
		QVector<QDomNode> alladd= getAddChild(pe);//список, чего можно добавить
		QVector<QDomNode> xlsc=getChild(pe); //список описаний детей
		if(xlsc.size()==0) return alladd;

		if(getXLSTypeRatio(xlsc.at(0))==sequence){
			//последователь
			QDomNode pos;
			if(!de.nextSiblingElement().isNull()) pos=de.nextSiblingElement();

			//ищем в списке данный
			int i=0;
			for(i=0;i<xlsc.count();i++){
				if(getNameXLS(xlsc.at(i))==de.nodeName()){
					break;
				}
			}

			//ищем в списке послодователя
			int j;
			if(pos.isNull()) j=xlsc.count()-1;
			else{
				for(j=i;j<xlsc.count();j++){
					if(getNameXLS(xlsc.at(j))==pos.nodeName()){
						break;
					}
				}
			}

			//идем вниз по списку, проверяя отношения текущего и наличие в списке допустимых
			for(int k=i;k<xlsc.count();k++){
				if(k<=j){//пока не дошли до последователя - добавляем все
					if(alladd.contains(xlsc.at(k))) vadd.append(xlsc.at(k));
				}else break;
			}
		}else{
			vadd=alladd;
		}

	return vadd;
}

/*
 * получить список имен детей элемента  по схеме, можно вставить после
 */
QStringList xmlschema::getInsAfterElementsName(QDomElement el){
	QVector<QDomNode> xlsc;
	QStringList rez;
	xlsc=getInsAfterElements(el);

	for(int i=0;i<xlsc.count();i++) rez<<getNameXLS(xlsc.at(i));
	return rez;
}

/*
 * Пытаемся конвертнуть маску в патерн ввода текстового редактор
 * Допустимы маски типа [0-9,A-F,a-f]{5}
 * В данном примере - получим патерн "HHHHH" (пять шестнадцатеричных чисел)
 * Допустимые типы - патернов - только цифры, только буквы, цифры+буквы, шестнадцатеричные цифры
 * Резделители - не поддерживаются (пока)
 */
QString xmlschema::convPat(const QString& str){
	QString rez;
	QString lit;
	bool ok;
	bool g=false;//цифры
	bool A=false;//все буквы
	bool HA=false;//A-F или a-f
	int n=1;//число вхождений

	QStringList sl=str.split(QRegExp("[\\[\\]]"));//до скобки, содержимое,после скобки


	//for(int i=0;i<sl.size();i++)qDebug()<<sl.at(i);
	//разбор маски
	if(sl.size()<3) return rez;
	rez.append(sl.at(0));
	if(sl.at(1).contains(QRegExp("0-9"))) g=true;
	if(sl.at(1).contains(QRegExp("A-F")) || sl.at(1).contains(QRegExp("a-f"))) HA=true;
	if(sl.at(1).contains(QRegExp("A-Z")) || sl.at(1).contains(QRegExp("a-Z"))) A=true;

	//Разбор кратности
	if(sl.at(2).size()>2){
		QStringList sn=str.split(QRegExp("[\\{\\}]"));//до скобки, содержимое,после скобки
		if(sn.size()==3){
			n=sn.at(1).toInt(&ok,10);
		}
	}

	if(g && !HA && !A)    	lit="g";//цифры
	else if(g && HA &&!A) 	lit="H";//шестнадцатиричное
	else if(A && !g && !HA) lit="A";//только буквы
	else if(A && g &&  !HA) lit="N";// буквы и цифры

	for(int i=0;i<n;i++) rez.append(lit);
	qDebug()<<rez;
	return rez;
}

/*
 * Поиск детей по имени внутри родителя не рекурсивно
 */
QVector<QDomNode> xmlschema::SearchChild(const QDomElement el,const QString& str){
	QVector<QDomNode> rez;
	QDomNodeList list=el.childNodes();
	for(int i=0;i<list.count();i++){
			if(str==list.at(i).nodeName()) rez<<list.at(i);
	}
	return rez;
}



int          xmlschema::getMinOccursXLS(QDomElement el){
	        //получить минимальную кратность элемента XLS
			return getMinOccursXLS(getNode(el));
}
int          xmlschema::getMaxOccursXLS(QDomElement el){
	       //получить ограничение кратности XLS (0- нет ограничения)
		    return getMaxOccursXLS(getNode(el));
}
NodeRatio    xmlschema::getXLSTypeRatioCild(QDomElement el){
	        //получить тип отношения элементов-потомков для родителя (вниз от родителя)
	         return getXLSTypeRatioCild(getType(el));
}
NodeRatio    xmlschema::getXLSTypeRatio(QDomElement el){
			//получить тип отношения элемента внутри группы (вверх от элемента)
	         return getXLSTypeRatio(getNode(el));
}

QString     xmlschema::getNameXLS(QDomNode el){
	        QString name=el.attributes().namedItem("name").nodeValue();//его имя
			if(name=="") name=el.attributes().namedItem("ref").nodeValue();
			return name;
}

bool xmlschema::getValid() {
	return Valid;
}
bool xmlschema::getValidDoc() {
	return ValidDoc;
}

