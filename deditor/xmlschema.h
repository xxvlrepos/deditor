/*
 * xmlschema.h
 *
 *  Created on: 22.05.2012
 *      Author: Usach
 *
  *
 *      Что пока не умеем:
 *      1. Множественность для отношений          (дублирует множественность элементов)
 *      2. Группы элементов и вложенные отношения - сделано, но требует проверки
 *
 */

#ifndef XMLSCHEMA_H_
#define XMLSCHEMA_H_
#include <QDomDocument>
#include <QDomNode>
#include <QMap>
#include <QtGui/QWidget>
#include <QtGui/QLineEdit>
#include <QtGui/QSpinBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QComboBox>
 #include <QtGui/QDateEdit>
#include "hexspinbox/hexspinbox.h"

enum NodeRatio { //отношения
	sequence = 0,//последовательность
    all,         //произвольный порядок
    choice       //один из списка
};

enum typeUseAttr {
	useRequired=1,
	useOtional=2,
	useProhibited=3
};

enum getTypeAttr{
	getAll=0,
	getRequired=1,
	getOtional=2
};


class xmlschema {
public:
	xmlschema(const QString& xsdName,QDomDocument* xml);
	virtual ~xmlschema();

	bool getValid();   //валидность схемы
	bool getValidDoc();//валидность документа

	QDomDocument* xmldoc;
	QDomNode  rootxml;
    //для узла hml (источник - узел документа)
	QDomNode     getNode(QDomElement);     //получить описание элемента по схеме xls
	QDomNode     getType(QDomElement);     //получить описание типа  элемент по схеме xls
	QVector<QDomNode> getChild(QDomElement);    //получить список описаний xls всех входящих елементов
	QVector<QDomNode> getAttributes(QDomElement,getTypeAttr type=getAll);//получить список описаний xls атрибутов
	QWidget*     getAttributeWidget(QDomAttr*,QWidget *parent=0);//получить виждет для редактирования атрибута
	QString defaultXML(); //получить дефолтный документ (фактичски - заголовок)

    //для имеющегося элемента документа
	int          getMinOccursXLS(QDomElement);        //получить минимальную кратность элемента XLS
	int          getMaxOccursXLS(QDomElement);       //получить ограничение кратности XLS (0- нет ограничения)
	NodeRatio    getXLSTypeRatioCild(QDomElement);   //получить тип отношения элементов-потомков для родителя (вниз от родителя)
	NodeRatio    getXLSTypeRatio(QDomElement);       //получить тип отношения элемента внутри группы (вверх от элемента)

	//для контроля допустимости относительно имеющегося элемента
	QStringList getChildName(QDomElement);    //получить список имен допустимых входящих елементов
	QStringList getAddChildName(QDomElement); //получить список имен  всех входящих елементов,которые еще можно добавить
	QStringList getReplaiceName(QDomElement);//получить список имен всех входящих елементов, на которые можно заменить данный (для отношения "или")
	QStringList getInsBeforeElementsName(QDomElement); //список имен котрые могут быть до
	QStringList getInsAfterElementsName(QDomElement);  //список имен котрые могут быть после
	QStringList getAttributesName(QDomElement,getTypeAttr type=getAll);//получить список имен допустимых атрибутов

	//Для найденного узла xls схемы (источник - узел схемы)
	int          getMinOccursXLS(QDomNode);        //получить минимальную кратность элемента XLS
	int          getMaxOccursXLS(QDomNode);       //получить ограничение кратности XLS (0- нет ограничения)
	NodeRatio    getXLSTypeRatioCild(QDomNode);   //получить тип отношения элементов-потомков для родителя (вниз от родителя)
	NodeRatio    getXLSTypeRatio(QDomNode);       //получить тип отношения элемента внутри группы (вверх от элемента)
	QString 	 getAttributeXLSDefault(QDomNode); //получить значение по умолчанию для узла XLS (не xml !!!)
	QString      getNameXLS(QDomNode);              //получить имя элемента схемы


protected:

	QDomNode     getTypeXLS(QDomElement);          //получить тип для узла схемы

	QVector<QDomNode> getAddChild(QDomElement); //получить список описаний xls всех входящих елементов,которые еще можно добавить
	QVector<QDomNode> getReplaice(QDomElement);//получить список описаний xls всех входящих елементов, на которые можно заменить данный (для отношения "или")
	QVector<QDomNode> getInsBeforeElements(QDomElement); //список элементов котрые могут быть до
	QVector<QDomNode> getInsAfterElements(QDomElement);  //список элементов котрые могут быть после

	QString      convPat(const QString&);//попытка конвертации патерна в маску ввода (см описание)
private:
	QVector<QDomNode> getElementXLS(const QDomElement); //ищем рекурсивно все описания элементов внутри типа, отношений или группы
	QVector<QDomNode> SearchChild(const QDomElement,const QString&);//дети по имени не рекурсивно
	QVector<QDomNode> getAttributesXLS(QDomNode, getTypeAttr);//Получить все атрибуты из найденного описания элемента в схеме
	typeUseAttr getTypeUseAttr(QDomNode); //определить тип использования атрибута
	 QDomDocument xsddoc;
	 QString namesp,namespp;
	 QDomNode  rootxsd; //начальный элемент схемы
	 QDomNode  rootHmlToXsd;//описание корневого элемента документа по схеме
	 QString FirstElName; //имя первого елемента (по схеме)
	 QString SxsdName;    //короткое имя схемы
	 bool Valid;          //валидность схемы
	 bool ValidDoc;       //валидность документа

	 //словари описаний автономных типов
	 QMap<QString,QDomNode> complexTypeMap;
	 QMap<QString,QDomNode> simpleTypeMap;
	 QMap<QString,QDomNode> attributeMap;
	 QMap<QString,QDomNode> attributeGroupMap;
	 QMap<QString,QDomNode> groupMap;
	 QMap<QString,QDomNode> elementMap;
};

#endif /* XMLSCHEMA_H_ */
