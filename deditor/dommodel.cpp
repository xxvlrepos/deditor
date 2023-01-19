/*
 * dommodel.cpp
 *
 *  Created on: 16.05.2012
 *      Author: Usach
 */

#include <QtGui>
#include <QtXml>

#include "domitem.h"
#include "dommodel.h"

DomModel::DomModel(xmlschema* schem, QObject *parent): QAbstractItemModel(parent)
{
	schema=schem;
	rootItem =  new DomItem(*(schema->xmldoc), 0);
	MapIcons=NULL;
	MapIconsDis=NULL; //иконки выключенных узлов по имени
	MapIconsEn=NULL;  //иконки включенных узлов по имени
	MapStatus=NULL;
	IdAttr=NULL;
}

DomModel::~DomModel()
{
    delete rootItem;
}

int DomModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 3;
}


QString DomModel::getNameToMask(const QDomNode node,bool noempty) const{
	QDomNamedNodeMap attributeMap = node.attributes();
	QString name;
	QString tx=node.nodeName();
	//для данного узла
	if(MapMask->contains(tx)){
		for(int i=0;i<MapMask->value(tx).size();i++){     //значение по маске атрибутов
			if(attributeMap.contains(MapMask->value(tx).at(i))){
				name=attributeMap.namedItem(MapMask->value(tx).at(i)).nodeValue().trimmed();
				if(name!="" && name!="0" ){
					break; //return name;
				}
			}
		}
	}

	//для любого узла
	if(name=="" && MapMask->contains("")){
		for(int i=0;i<MapMask->value("").size();i++){     //значение по маске атрибутов
			if(attributeMap.contains(MapMask->value("").at(i))){
				name=attributeMap.namedItem(MapMask->value("").at(i)).nodeValue().trimmed();
				if(name!="" && name!="0" ){
					break;//return name;
				}
			}
		}
	}
	if(noempty && (name=="" || name=="0")) name=tx;
	if(AddMask->contains(tx)){
		for(int i=0;i<AddMask->value(tx).size();i++){
			if(attributeMap.contains(AddMask->value(tx).at(i))){
				name+=(SepMask->value(tx)+
						attributeMap.namedItem(AddMask->value(tx).at(i)).nodeValue().trimmed());
			}
		}
	}
	return name;
}

QVariant DomModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())        return QVariant();
    QString tx="";

	DomItem *item = static_cast<DomItem*>(index.internalPointer());
	QDomNode node = item->node();
	QString name;

    switch(role){
    	case Qt::DisplayRole:
			switch (index.column()) {
				case 0:
					return getNameToMask(node);
				case 1:
					return tx.setNum(item->nodeType,10);//тип узла
				case 2:
					return item->value;                 //содержимое узла
				default:
					return QVariant();
			}

		case Qt::DecorationRole:
			int status=0;
			if(index.column()==0){
				name=node.nodeName();

				if(MapStatus!=NULL){
					if( IdAttr!=NULL){//по идентификатору - атрибуту
						status=MapStatus->value(node.toElement().attribute(*IdAttr,""),0);
						if(status>0) status= 1;
						if(status<0) status=-1;
					}
					if(status==0 && !node.parentNode().isNull()){ //по имени "родитель,узел,номер"
						if(!node.parentNode().isNull()){
							status=MapStatus->value(node.parentNode().nodeName()+","+name+","+tx.setNum(index.row()));
						}
					}
				}

				switch(status){
					case 0:
						if(MapIcons!=NULL){
							if(MapIcons->contains(name)) return (QIcon)MapIcons->value(name);
						}
						break;
					case -1:
						if(MapIconsDis!=NULL){
							if(MapIconsDis->contains(name)) return (QIcon)MapIconsDis->value(name);
						}
						break;
					case 1:
						if(MapIconsEn!=NULL){
							if(MapIconsEn->contains(name)) return (QIcon)MapIconsEn->value(name);
						}
						break;
				}
		   }
    }
    return QVariant();
}


Qt::ItemFlags DomModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}


QVariant DomModel::headerData(int section, Qt::Orientation orientation,int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return tr("Name");
            case 1:
                return tr("Type");
            case 2:
                return tr("Value");
             default:
                return QVariant();
        }
    }

    return QVariant();
}

QModelIndex DomModel::index(int row, int column, const QModelIndex &parent)  const
{
	if (!hasIndex(row, column, parent))  return QModelIndex();

    DomItem *parentItem;

    if (!parent.isValid())  parentItem = rootItem;
    else			        parentItem = static_cast<DomItem*>(parent.internalPointer());

    DomItem *childItem = parentItem->child(row);
    if (childItem)  return createIndex(row, column, childItem);
    else            return QModelIndex();
}

QModelIndex DomModel::parent(const QModelIndex &child) const
{
    if (!child.isValid())    return QModelIndex();

    DomItem *childItem = static_cast<DomItem*>(child.internalPointer());
    DomItem *parentItem = childItem->parent();

    if (!parentItem || parentItem == rootItem) return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}


int DomModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;

    DomItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<DomItem*>(parent.internalPointer());

    /*
    if(parentItem->nodeType==ELEMENT){//не показываем текстовые элементы
    	return 0;
    }
    */
    return parentItem->node().childNodes().count();
}

DomItem* DomModel::nodeFromIndex(const QModelIndex& index) const {
	if (index.isValid()) {
        return static_cast<DomItem*>(index.internalPointer());
    }else{
        return rootItem;
    }
}
/*
 * Удалить узел из дерева (и подчиненные)
 */
bool DomModel::removeRows ( int row, int count, const QModelIndex & parent){
	DomItem* item=nodeFromIndex(parent);
	if(count<=0)return false;
	if(item==rootItem) return false; //нельзя удалить первый уровень
	if((row+count)>(rowCount(parent))) return false; //нельзя удалить то - чего нет

	beginRemoveRows(parent,row,row+count-1);

			for(int i=row;i<row+count;i++){
				QDomNode del=item->child(i)->node();
				item->node().removeChild(del);
			}

			item->deleteChild(row);
			for(int i=row;i<rowCount(parent);i++){
				index(i,0,parent);
			}

	endRemoveRows();
	return true;
}

/*
 * Добавить узел
 */

bool DomModel::insertRows (int row, int count, const QModelIndex & parent,getTypeAttr typeAt){

	DomItem* item=nodeFromIndex(parent);
	if(item==rootItem || count<=0) return false; //нельзя добавить на первый уровень

	QDomElement de=item->node().toElement();
	if(newEl.isNull() || de.isNull()) return false;

	beginInsertRows(parent,row,row+count-1);
	QDomElement ne=newEl;
	for(int i=0;i<count;i++){

		int n=rowCount(parent);//уже есть элементы
		if(row>n) row=n;
		if(n==0 )      de.appendChild(ne);
		else if(row>0) de.insertAfter(ne,item->child(row-1)->node());
		else           de.insertBefore(ne,item->child(row)->node());
		//добавление атрибутов
		QVector<QDomNode> al=schema->getAttributes(ne,typeAt); //список атрибутов
		for(int j=0;j<al.count();j++){//добавляем атрибуты
			ne.setAttribute(al.at(j).attributes().namedItem("name").nodeValue(),schema->getAttributeXLSDefault(al.at(j)));
		}

		item->addChild(row);

		newIn=index(row, 0, parent);//его индекс
		for(int j=row+1;j<rowCount(parent);j++){
			index(j,0,parent);
		}
		row++;
		QDomElement ne=newEl.cloneNode().toElement();//копия для повторной вставки
	}

	endInsertRows();
	return true;
}



void DomModel::setNewEl(QDomElement& ne){
	newEl=ne;
}

QModelIndex DomModel::getNewIn(){
	return newIn;
}

