/*
 * domattr.cpp
 *
 *  Created on: 17.05.2012
 *      Author: Usach
 */

#include "datatable.h"
#include <QtCore>
#include <QtGui/QLineEdit>
#include <QtGui/QSpinBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QComboBox>
#include "hexspinbox/hexspinbox.h"



DataTable::DataTable(QList<QDomNode>& node,QStringList& nameL,QObject *parent): QAbstractTableModel(parent)
{
	// TODO Auto-generated constructor stub
	srcL=node;
	bool fn,ok;
	if(srcL.size()==0) return;
	ur=0;
	if(srcL.size()>1) ur=1;

	for(int j=0;j<srcL.size();j++){
		QString Name;
		if(j<nameL.size()) Name=nameL.at(j);

		QDomNodeList op=srcL.at(j).childNodes();
		fn=true; //можно добавить имя канала (однократно)
		for(int i=0;i<op.count();i++){
			if(op.at(i).toElement().attributeNode("read").value().toInt(&ok,10)>ur){
				list<<op.at(i);
				if(fn){
					NameK<<Name;
					fn=false;
				}else     NameK<<""; //не дублируем имена!
			//	NameT<<srcL.at(j).nodeName();
				Data<<op.at(i).toElement().attributeNode("name").value();
				Value<<"";
				Dim<<op.at(i).toElement().attributeNode("dimension").value();
			}
		}
	}
}

DataTable::~DataTable() {
	// TODO Auto-generated destructor stub
}


int DataTable::rowCount(const QModelIndex &) const
{
	if(srcL.size()==0) return 0;
	return list.size();
}

int DataTable::columnCount(const QModelIndex &) const
{
	return 4;
}



QVariant DataTable::headerData(int section, Qt::Orientation orientation,int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                 return tr("Name");
            case 1:
                return tr("Data");
            case 2:
                return tr("Value");
            case 3:
            	return tr("Dim");
            default:
                return QVariant();
        }
    }

    return QVariant();
}

QVariant DataTable::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) 		 return QVariant();

	switch(role){
		case Qt::DisplayRole:
		case Qt::EditRole:
			switch (index.column()) {
				case 0:
					return NameK.at(index.row());
				case 1:
					return Data.at(index.row());
				case 2:
					return Value.at(index.row());
				case 3:
					return Dim.at(index.row());
			   default:
					return QVariant();
			}
	}

		return QVariant();
}


Qt::ItemFlags DataTable::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}


int DataTable::size(){
	return list.size();
}

QModelIndex DataTable::index(int row, int column, const QModelIndex &parent)  const
{
	if (!hasIndex(row, column, parent))  return QModelIndex();
	return createIndex(row, column);
}

bool DataTable::setData(const QModelIndex &index,const QVariant& value,int role)
{
	if(index.isValid() && (role==Qt::EditRole || role==Qt::DisplayRole)){
		 switch(index.column()){
			 case 0:
				 return false;//имя атрибута не меняем
			 case 1:
				 return false;//имя атрибута не меняем
			 case 2:
				 Value[index.row()]=value.toString();
				 break;
			 case 3:
				 return false;

		 }
		 return true;
	}
	return false;
}

void DataTable::setInfo(int i,QString z){
	QModelIndex ind=index(i,2);
	beginResetModel();
	setData(ind, z);
	endResetModel();

}





