/*
 * dommodel.h
 *
 *  Created on: 16.05.2012
 *      Author: Usach
 */

#ifndef DOMMODEL_H_
#define DOMMODEL_H_
#include <QAbstractItemModel>
#include <QDomDocument>
#include <QModelIndex>
#include <QVariant>
#include <QMap>
#include "xmlschema.h"

class DomItem;

class  DomModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    DomModel(xmlschema*, QObject *parent = 0);
    ~DomModel();

    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    DomItem*    nodeFromIndex(const QModelIndex& index) const;//узел по индексу

    bool insertRows ( int row, int count, const QModelIndex & parent = QModelIndex(),getTypeAttr typeAt=getAll);
    bool removeRows ( int row, int count, const QModelIndex & parent = QModelIndex());
    void setNewEl(QDomElement& newEl);
    QModelIndex getNewIn();
    void changeDataM(QModelIndex index){emit dataChanged(index,index);};
    QString getNameToMask(const QDomNode,bool noempty=true) const;

    QMap<QString,QIcon>    *MapIcons;
    QMap<QString,QIcon>    *MapIconsDis; //иконки выключенных узлов по имени
    QMap<QString,QIcon>    *MapIconsEn;  //иконки включенных узлов по имени
    QHash<QString,int> *MapStatus;
    QMap<QString, QStringList> *MapMask; //словарь приорететных замен имен узлов на имена атрибутов
    QMap<QString, QStringList> *AddMask; //словарь добав. имен узлов на имена атрибутов
    QMap<QString, QString> *SepMask; //сепараторы
    QString			   *IdAttr;

private:
    xmlschema* schema;
    DomItem *rootItem;
    QDomElement newEl;//куда кладем элемент для добавления
    QModelIndex newIn;//откуда получаем его индекс


};

#endif /* DOMMODEL_H_ */
