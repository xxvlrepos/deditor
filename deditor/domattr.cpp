/*
 * domattr.cpp
 *
 *  Created on: 17.05.2012
 *      Author: Usach
 */

#include "domattr.h"
#include <QtCore>
#include <QtGui/QLineEdit>
#include <QtGui/QSpinBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QComboBox>
#include "hexspinbox/hexspinbox.h"



DomAttr::DomAttr(DomItem* it,xmlschema* sch,QObject *parent): QAbstractTableModel(parent)
{
	// TODO Auto-generated constructor stub
	item=it;
	schema=sch;
	AllColored=false;
}

DomAttr::~DomAttr() {
	// TODO Auto-generated destructor stub
}


int DomAttr::rowCount(const QModelIndex &) const
{
	if(!item) return 0;
	return item->node().attributes().count();

}

int DomAttr::columnCount(const QModelIndex &) const
{
	return 2;
}



QVariant DomAttr::headerData(int section, Qt::Orientation orientation,int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return tr("Name attribute");
            case 1:
                return tr("Value");
            default:
                return QVariant();
        }
    }

    return QVariant();
}

QVariant DomAttr::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) 		 return QVariant();
    if(!item) 					 return QVariant();

	switch(role){
		case Qt::DisplayRole:
		case Qt::EditRole:
			switch (index.column()) {
				case 0:
					return item->node().attributes().item(index.row()).nodeName();
				case 1:
					return item->node().attributes().item(index.row()).nodeValue();
			}
		case Qt::BackgroundColorRole:
			  switch (index.column()) {
				case 0:
					return color;
				case 1:
					QString name=item->node().attributes().item(index.row()).nodeName();
					if((!AllColored && ColoredList.contains(name)) ||
					   (AllColored && !ColoredList.contains(name))
					) return color;
			  }
	}

	return QVariant();
}


Qt::ItemFlags DomAttr::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return 0;
    if(index.column()==1) return Qt::ItemIsEnabled | Qt::ItemIsSelectable  | Qt::ItemIsEditable;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool DomAttr::setData(const QModelIndex &index,const QVariant& value,int role)
{
	if(!item) 	return false;
	if(index.isValid() && (role==Qt::EditRole || role==Qt::DisplayRole)){

		 switch(index.column()){
			 case 0:
				 return false;//имя атрибута не меняем
			 case 1:
				 item->node().attributes().item(index.row()).setNodeValue(value.toString());
				 break;
		 }
		 return true;
	}
	return false;
}

DomItem* DomAttr::getParentItem(){
	return item;
}

xmlschema* DomAttr::getSchema(){
	return schema;
}

QDomNode DomAttr::DomNodeFromIndex(const QModelIndex& index) const {
	 if(!index.isValid()) 		 return QDomNode();
	 if(!item) 					 return QDomNode();
	 return item->node().attributes().item(index.row());
}

void DomAttr::setColorList(QColor col,bool all,QString str){
	 color=col;
	 AllColored=all;
	 ColoredList=str.split(",");
}

void DomAttr::addToColorList(QString str) {ColoredList<<str;}

//--------------------------------------------
MyDelegate::MyDelegate(QObject *parent): QItemDelegate(parent) {
	OldData="";
	Editable=true;
	connect(this, SIGNAL(oldDate(QString)),this, SLOT(SaveOldDate(QString)));
}

QWidget *MyDelegate::createEditor(
            QWidget *parent,
            const QStyleOptionViewItem& /* option */,
            const QModelIndex&  index) const
{

	QWidget *editor;

	DomAttr* da=(DomAttr*)index.model();//модель данных
	if(da->getSchema()==0) {//есть схема или нет
		    if(Editable) editor = new QLineEdit(parent);
		    else         editor = new QWidget(parent);
	}else{//тип редактора ищем по схеме
			//имя атрибута
			QString name=index.model()->data(index.sibling(index.row(),0), Qt::DisplayRole).toString();
			// сам атрибут в DOM
			QDomAttr att=da->getParentItem()->node().attributes().namedItem(name).toAttr();

			//тип редактора
			if( (Editable && !NoEdList.contains(name)) ||
				(!Editable && EdList.contains(name))){
				 editor = da->getSchema()->getAttributeWidget(&att,parent);
			}else  editor = new QWidget(parent);
	}
    editor->installEventFilter(const_cast<MyDelegate*>(this));
    //editor->setStyleSheet(QString("QLineEdit{ background-color: %1 }").arg(QColor(230,250,255).name()));

    return editor;
}

void MyDelegate::setEditorData(
		QWidget *editor,
		const QModelIndex& index) const
{
    bool ok;
    int ind=-1;
    QString t = index.model()->data(index, Qt::EditRole).toString();

    emit oldDate(t);

    if (QLineEdit * le = qobject_cast<QLineEdit *>(editor)){
    	le->setText(t);
    }
    if (QSpinBox * sb = qobject_cast<QSpinBox *>(editor)){
        sb->setValue(t.toInt(&ok,10));
    }
    if (HexSpinBox * hsb = qobject_cast<HexSpinBox *>(editor)){
            hsb->setValue(t.toInt(&ok,16));
    }
    if (QDoubleSpinBox * dsb = qobject_cast<QDoubleSpinBox *>(editor)){
        dsb->setValue(t.toFloat(&ok));
    }
    if (QComboBox * cb = qobject_cast<QComboBox *>(editor)){
    	if(!cb->isEditable()){
			ind=cb->findText(t);
			if(ind>-1) cb->setCurrentIndex(ind);
    	}else{
    		cb->setEditText(t);
    	}
    	//connect(cb, SIGNAL(currentIndexChanged(int)),this,SLOT(setCombo(int)));
    }
    if (QDateEdit * de = qobject_cast<QDateEdit *>(editor)){
    	 QLocale loc(QLocale::English, QLocale::UnitedStates);
    		 de->setDate(loc.toDateTime(t,"yyyy-MM-dd").date());
    		 ok=true;
    }

}


void MyDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	QString t;
	bool ok=false;

	if (QLineEdit * le = qobject_cast<QLineEdit *>(editor)){
		t = le->text();
		ok=true;
	}
	if (QSpinBox * sb = qobject_cast<QSpinBox *>(editor)){
		t.setNum(sb->value(),10);
		ok=true;
	}
	if (HexSpinBox * hsb = qobject_cast<HexSpinBox *>(editor)){
		t.setNum(hsb->value(),16);
		t=t.toUpper();
		if((t.count()/2.0-t.count()/2)>0) t="0"+t;
		ok=true;
	}
	if (QDoubleSpinBox * dsb = qobject_cast<QDoubleSpinBox *>(editor)){
		t.setNum(dsb->value(),'f',dsb->decimals());
		ok=true;
	}
	if (QComboBox * cb = qobject_cast<QComboBox *>(editor)){
		 t=cb->currentText();
		 ok=true;
	}
	if (QDateEdit * de = qobject_cast<QDateEdit *>(editor)){
		 t=de->date().toString("yyyy-MM-dd");
		 ok=true;
	}

	if(!ok) return;


    model->setData(index, t, Qt::EditRole);
    if(OldData!=t){
    	emit changeData();
    }
}

void MyDelegate::updateEditorGeometry(
            QWidget *editor,
            const QStyleOptionViewItem &option,
            const QModelIndex& /* index */) const
{
    editor->setGeometry(option.rect.adjusted(-1,-1,1,1));
}

void MyDelegate::SaveOldDate(QString str){
	OldData=str;
}

void MyDelegate::setEditableAll(bool ed){
	Editable=ed;
}


void MyDelegate::setEdList(QString str){
	EdList=str.split(",");
}
void MyDelegate::setNoEdList(QString str){
	NoEdList=str.split(",");
}

void MyDelegate::addEdList(QString ads){
	EdList<<ads;
}
void MyDelegate::addNoEdList(QString ads){
	NoEdList<<ads;
}
