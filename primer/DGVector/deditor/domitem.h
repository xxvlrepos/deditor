/*
 * domitem.h
 *
 *  Created on: 16.05.2012
 *      Author: Usach
 */

#ifndef DOMITEM_H_
#define DOMITEM_H_

#include <QDomNode>
#include <QHash>

enum NodeType {
    DEFAULT = 0,//прочий
    GROUPEL,      //элемент -группа -содержит внутри другие элементы (не текст)
    ELEMENT,    //конечный элемент с значением (внутри только текст)
    EMPTY,      //пустой конечный элемент (только атрибуты)
    TEXT        //текст
};

class DomItem
{
public:
    DomItem(QDomNode &node, int row, DomItem *parent = 0);
    ~DomItem();
    DomItem* child(int);
    DomItem* parent();
    QDomNode node() const;
    int row();
    NodeType nodeType;
    QString	 value;

    void clearChild();
    void addChild(int);
    void deleteChild(int);


private:
    QDomNode domNode;
    QHash<int,DomItem*> childItems;
    DomItem *parentItem;
    int rowNumber;
};

#endif /* DOMITEM_H_ */
