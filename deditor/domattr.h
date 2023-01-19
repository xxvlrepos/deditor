/*
 * domattr.h
 *
 *  Created on: 17.05.2012
 *      Author: Usach
 */

#ifndef DOMATTR_H_
#define DOMATTR_H_

#include "domitem.h"
#include "xmlschema.h"
#include <QAbstractTableModel>
#include <QItemDelegate>


class DomAttr: public QAbstractTableModel {
	  Q_OBJECT
public:
	DomAttr(DomItem* item = 0,xmlschema* schema=0,QObject* parent = 0);
	~DomAttr();

	 int rowCount(const QModelIndex &parent=QModelIndex()) const;
	 int columnCount(const QModelIndex &parent=QModelIndex()) const;

	 QVariant data(const QModelIndex &index, int role) const;
	 Qt::ItemFlags flags(const QModelIndex &index) const;
	 QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
	 bool setData(const QModelIndex &index,const QVariant& value,int role = Qt::DisplayRole);

	 DomItem* getParentItem();//получить ссылку на элемент - хозяин атрибутов в DOM
	 xmlschema* getSchema();//на вычислитель типа делегата и проч - работа со схемой

     //для работы контекстного меню
	 QDomNode DomNodeFromIndex(const QModelIndex& index) const; //получить атрибут в DOM по индексу

	 void setColorList(QColor col,bool all=true,QString str="");
	 void addToColorList(QString);

private:
	 DomItem*   item;
	 xmlschema* schema;
	 bool         AllColored; //красим всех или по списку
     QColor       color;
	 QStringList  ColoredList; //подсветка фона
};


//Делегат для редактирования таблицы
//------------------------------------------------------------------------------
class MyDelegate: public QItemDelegate {
    Q_OBJECT
public:
    MyDelegate(QObject *parent = 0);
    ~MyDelegate(){};

    QWidget *createEditor(
                QWidget *parent,
                const QStyleOptionViewItem &option,
                const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor,const QStyleOptionViewItem &option,const QModelIndex &index) const;

    void setEditableAll(bool);//включить/выключить редактирование атрибутов
    void setEdList(QString);
    void setNoEdList(QString);
    void addEdList(QString);
    void addNoEdList(QString);

   signals:
      void oldDate(QString) const;
	  void changeData() const;

private:

	QString OldData;//что было в ячейке до редактирования
	bool Editable;
	QStringList  EdList;
	QStringList  NoEdList;

private	slots:
	void SaveOldDate(QString);
};

#endif /* DOMATTR_H_ */
