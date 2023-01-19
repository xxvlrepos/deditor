#ifndef DEDITOR_H
#define DEDITOR_H

#include <QtCore>
#include <QtGui/QWidget>
#include <QStringList>
#include "ui_deditor.h"
#include <QSortFilterProxyModel>
#include <QAbstractProxyModel>
#include "dommodel.h"
#include "domattr.h"
#include "xmlschema.h"
#include <const.h>

class deditor : public QWidget
{
    Q_OBJECT

public:
    deditor(const QString& xsdName,const QString& xmlName="",QWidget *parent = 0);
    ~deditor();

    static const int DeleteNode=1;
    static const int AddNode=2;
    static const int ClearNode=4;
    static const int AddBefore=8;
    static const int AddAfter=16;
    static const int ReplaceNode=32;
    static const int AddAttribute=64;
    static const int DeleteAttribute=128;

    static bool xmlValidator(const QString& xsdName, const QString& Name,QDomDocument* doc=NULL);

    Ui::deditorClass ui; //виджеты доступны для управления с наружи (добавления своих элементов)
    QDomElement rootNode;//
    xmlschema* schema;
    QDomDocument document;

    bool setDocument(const QString& xmlName=""); //открыть документ, если не указан присоздании класса
    bool saveDomAs(const QString&);
    bool saveCurDom();
    bool getChangeData();      //было-ли не сохраненное изменение данных?


    QModelIndex indexFromNode(QDomElement);//ищем индекс в модели по элементу
    QModelIndex indexTreeFromNode(QDomElement);//ищем индекс дереве по элементу (разница в ProxyModel)

    //надо указать, относительно какого узла вставляем, для этого используем indexTreeFromNode
    void deleteNode(const QModelIndex&);		//удалить  узел
    void clearNode(const QModelIndex&);         //очистить узел
    QDomElement addNode(const QModelIndex&,const QString&);   //добавить  узел по имени
    void addAttribute(const QModelIndex&,const QString&,const QString& v="");     //добавить атрибут по имени
    void deleteAttribute(const QModelIndex&,const QString&);  //удалить атрибут по имени
    void replaceNode(const QModelIndex&,const QString&);       //заменить узел по имени
    void insertNodeBefore(const QModelIndex&,const QString&);  //вставка узла в позицию до
    void insertNodeAfter(const QModelIndex&,const QString&);   //вставка узла в позицию после
    void goParent(const QModelIndex&);                        //програмно перейти на родителя (например-перед удалением)
    void goElement(const QDomElement);                        //програмно перейти на элемент

    //списки допустимых имен узлов для операции над заданным в дереве (indexTreeFromNode)
    QStringList addNodeList(const QModelIndex&); //для родителя - список имен - можно добавить
    QStringList clearNodeList(const QModelIndex&); //для родителя - список имен, которые можно удалить из родителя
    QStringList replaceNodeList(const QModelIndex&);//на какие можно заменить (или-или)
    QStringList insertNodeBeforeList(const QModelIndex&);//что можно вставить до
    QStringList insertNodeAfterList(const QModelIndex&);//что можно вставить после
    QStringList addAttributeList(const QModelIndex&);  //какие атрибуты еще можно добавить
    QStringList clearAttributeList(const QModelIndex&);//какие атрибуты можно удалить
    int        isDeleted(const QModelIndex&); //можно удалить (по индексу)?
    int        isDeleted(const QDomElement);   //можно удалить (по элементу)?

    //Управление контекстным меню + добавлям свои действия
    //управление отображением и добавлением узлов
    void setEditableFromMenu(bool);//разрешает в меню пункты редактирования документа в соответствии с правилами схемы
    QMap<QString,int>               EdContextMenu;  //маска пунктов контекстного меню кот добавляем
    QMap<QString,int>               NoEdContextMenu;//маска пунктов контекстного меню которые убираем
    QMap<QString,QList<QAction*> >  ExtActionTree;//внешние акции в зависимости от имени узла дерева
    QMap<QString,QList<QAction*> >  ExtActionTable;//внешние акции в зависимости имени атрибута таблицы


    void setEditableAllAttribute(bool);//разрешить/запретить редактировать атрибуты (все) кроме см ниже.
    void setEdAttribute(QString);      //список имен атрибутов, кот можно редактировать через запятую без проб
    void setNoEdAttribute(QString);    //список имен атрибутов, кот нельзя редактировать через зап без проб
    void addEdAttribute(QString);      //добавить имя аттр в разр. список
    void addNoEdAttribute(QString);	   //добавить имя аттр в запрещ. список

    void setPolicyGetAttribute(bool);//при создании узла какие атрибуты добавлять (0-все, или 1-только обязательные)

    void setMask(QString,QStringList); //перечень атрибутов вместо имени узла в порядке уменьшения приоритетов
    void setMask(QString,QString);     //перечень атрибутов через запятую вместо имени узла в порядке уменьшения приоритетов
    void addMask(QString n,QStringList al,QString r=":"); //перечень атрибутов добавл к имени узла и разделитель
    void addMask(QString n,QString as,QString r=":");
    QString getNameMask(QDomElement el,bool noempty=true){return model->getNameToMask(el,noempty);} //получить имя узла с учетом маски
    //иконки
    void setIcons(QString name,QString file,QString fileDis="",QString fileEn=""); //иконки для имен узлов
    QString                    IdAttr;//имя какого атрибута служит идентификатором
    QHash<QString,int>		   MapIdStatus;//статусы узлов -1,0,1 (выкл,неизв,вкл)

    void setIdAttr(QString id){IdAttr=id; MapIdStatus.clear();}//указываем какой атрибут будет  id
    void setIdStatus(QString id,int status);//задать новый статус id
    void clearIdSatus();//сбросить все статусы
    QModelIndex searchIDTreeIndex(const QString& id);//ищем по id в дереве
    QDomElement searchNodeID(const QString& id){return searchNodeID(rootNode,id);}
    //
    QDomNode getCurDomNode(void);//получить выделенный узел документа
    //
    QString getNameXml();//получить имя документа
    QString getNameXls();//получить имя схемы

public slots:
    void saveDom();								     //запись в файл
    void groupChanged(const QModelIndex& index); //смена узла - пречитываение атрибутов в таблице редактировани


signals:
    void sendMess(const QString&);  //результат выполнения команды (для отладки)
    void AttManualChange(QDomNode);  //ручное изменение атрибута произведено
    void AddManualNode(QDomElement);    //ручное добавление узла
    void changeData();              //сигнал - данные изменились
    void changeCurNode(QDomNode);   //выбран узел
    void deleteNode(QStringList);
    void isSave(); //изменения сохранены


protected:

    QMap<QString,QIcon>        MapIcons; //иконки для узлов по имени
    QMap<QString,QIcon>        MapIconsDis; //иконки выключенных узлов по имени
    QMap<QString,QIcon>        MapIconsEn;  //иконки включенных узлов по имени
    QMap<QString, QStringList> MapMask; //словарь приорететных замен имен узлов на имена атрибутов
    QMap<QString, QStringList> AddMask; //словарь приорететных замен имен узлов на имена атрибутов
    QMap<QString, QString> SepMask;

//для внутренних интерактивных вызовов
protected slots:
    void deleteNode();							//удалить выделенный  узел
    void clearNode();                           //очистить выделенный узел
    void addNode(const QString&);               //добавить в узел по имени - родитель берется по выделенному элементу
    void replaceNode(const QString&);           //заменить узел по имени - родитель берется по выделенному элементу
    void insertNodeBefore(const QString&);      //вставка узла в позицию до
    void insertNodeAfter(const QString&);       //вставка узла в позицию после
    void addAttribute(const QString&);         //добавить атрибут по имени
    void deleteAttribute(const QString&);      //удалить атрибут по имени
    void slotAttManualChange();                 //изменение ручное атрибута
    void showContextMenuForTreeView(const QPoint&); //генератор контекстного меню дерева
    void showContextMenuForTableView(const QPoint&); //генератор контекстного меню таблицы

private:

    QString xmlName,xlsName;


    DomModel                *model;
    QSortFilterProxyModel   *treeProxyModel; //фильтр для дерева
    DomAttr		            *tableAttr;//табличная модель для просмотра и исправления атрибутов

    bool                    DataChange; //признак изменения данных
    bool                    manualChange;


    //QSortFilterProxyModel *tableProxyModel;//отключено - фильтр для табличной части
    getTypeAttr getAttr;    //способ добавления атрибутов при добавлении элемента
    bool     EditableFromMenu;


    QAction* addAct;  //добавление узла
    QAction* delAct;  //удаление узла
    QAction* clearAct;  //очистка узла
    QSignalMapper* MapperAdd;
    QSignalMapper* MapperReplace;
    QSignalMapper* MapperBefore;
    QSignalMapper* MapperAfter;
    QSignalMapper* MapperAddAtt;
    QSignalMapper* MapperDelAtt;

    MyDelegate *delegate;

    bool deleteNodeModel(const QModelIndex&);
    bool addNodeModel(const QModelIndex&);
    int countChildElement(const QDomElement,const QString&);//считаем детей по имени
    void addEditToContextMenu(QMenu&,QModelIndex,int);
    QDomElement searchNodeID(QDomElement,const QString&);

    QColor           noEdColor;
    bool             AllColored;
    QStringList      ColoredList;
    QStringList      deletedId;

};


#endif
//**************************************************************************

