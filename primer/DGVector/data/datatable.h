/*
 * datatable.h
 *
 *  Created on: 17.05.2012
 *      Author: Usach
 */

#ifndef DATATABLE_H_
#define DATATABLE_H_

#include <QDomNode>
#include <QAbstractTableModel>
#include <QVector>
#include <QStringList>



class DataTable: public QAbstractTableModel {
	  Q_OBJECT
public:
	 DataTable(QList<QDomNode>& node, QStringList& nameL, QObject* parent = 0);
	~DataTable();

	 int rowCount(const QModelIndex &parent=QModelIndex()) const;
	 int columnCount(const QModelIndex &parent=QModelIndex()) const;
	 QModelIndex index(int row, int column,const QModelIndex &parent = QModelIndex()) const;
	 QVariant data(const QModelIndex &index, int role) const;
	 Qt::ItemFlags flags(const QModelIndex &index) const;
	 QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	 bool setData(const QModelIndex &index,const QVariant& value,int role = Qt::DisplayRole);

	 int size();
	 void setInfo(int i,QString);

private:
	 int ur;
	 QList<QDomNode> srcL;
	 QList<QDomNode> list;

	 QStringList NameK;
//	 QStringList NameT;
	 QStringList Data;
	 QStringList Value;
	 QStringList Dim;
};


#endif /* DOMATTR_H_ */
