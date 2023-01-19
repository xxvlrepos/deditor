/*
 * domitem.cpp
 *
 *  Created on: 16.05.2012
 *      Author: Usach
 */

#include <QtXml>

#include "domitem.h"


DomItem::DomItem(QDomNode &node, int row, DomItem *parent)
{
    domNode = node;
    rowNumber = row;
    parentItem = parent;
    nodeType=DEFAULT;

    switch(domNode.nodeType()){
		case QDomNode::TextNode:
			 nodeType=TEXT;
			 value=domNode.nodeValue();
			 break;
		case QDomNode::ElementNode:
			switch(domNode.childNodes().count()){
				case 0: nodeType=EMPTY; break;
				case 1:
					if(domNode.childNodes().at(0).nodeType()==QDomNode::TextNode){
						value=domNode.childNodes().at(0).nodeValue();
						nodeType=ELEMENT;
					}else nodeType=GROUPEL;
					break;
				default: nodeType=GROUPEL;
			}
			break;
		default:
			nodeType=DEFAULT;
    }
    //qDebug()<<domNode.nodeName()+":"+value;
}

DomItem::~DomItem()
{
    QHash<int,DomItem*>::iterator it;
    for (it = childItems.begin(); it != childItems.end(); ++it)  delete it.value();
}

QDomNode DomItem::node() const
{
    return domNode;
}

DomItem *DomItem::parent()
{
    return parentItem;
}

DomItem *DomItem::child(int i)
{
	if (childItems.contains(i))  return childItems[i];

    if (i >= 0 && i < domNode.childNodes().count()) {
        QDomNode childNode = domNode.childNodes().item(i);
        DomItem *childItem = new DomItem(childNode,i, this);
        childItems[i] = childItem;
        return childItem;
    }
    return 0;
}

int DomItem::row()
{
    return rowNumber;
}

void DomItem::clearChild(){
	 QHash<int,DomItem*>::iterator it;
	    for (it = childItems.begin(); it != childItems.end(); ++it)  delete it.value();
	childItems.clear();
}

void DomItem::addChild(int n){
	int cou=childItems.count();
	if(n>cou) n=cou;
	if(childItems.contains(n)){
		for(int i=n;i<cou;i++){
			delete childItems[i];
			childItems.remove(i);
		}
	}
}


void DomItem::deleteChild(int n){
	 int cou=childItems.count();
	 if(n>=cou) n=cou-1;

	for(int i=n;i<cou;i++){
		delete childItems[i];
		childItems.remove(i);
	}
}




