#include "deditor.h"
#include <QtGui/QMenu>
#include <QXmlSchema>
#include <QXmlSchemaValidator>



deditor::deditor(const QString& xsdStr,const QString& xmlStr,QWidget *parent): QWidget(parent)
{

   schema=NULL;
   model=NULL;
   DataChange=false;
   manualChange=false;
   noEdColor=QColor(240,240,240);
   AllColored=false;

   if(xmlStr==""){
    	schema = new xmlschema(xsdStr,&document);
    	if(!schema->getValid()) return;
    	rootNode=document.documentElement();
    }

	DataChange=false;
	EditableFromMenu=true;
	getAttr=getAll;
	tableAttr=NULL;
	MapperAdd= NULL;
	MapperReplace=NULL;
	MapperBefore=NULL;
	MapperAfter=NULL;
	MapperAddAtt=NULL;
	MapperDelAtt=NULL;

	ui.setupUi(this);


	delAct = new QAction(tr("&Delete node"),this);
	connect(delAct, SIGNAL(triggered()),this,SLOT(deleteNode()));

	clearAct = new QAction(tr("&Clear node"),this);
	connect(clearAct, SIGNAL(triggered()),this,SLOT(clearNode()));

	ui.treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.treeView, SIGNAL(customContextMenuRequested(const QPoint &)),this, SLOT(showContextMenuForTreeView(const QPoint &)));
	ui.tableView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(ui.tableView, SIGNAL(customContextMenuRequested(const QPoint &)),this, SLOT(showContextMenuForTableView(const QPoint &)));


	treeProxyModel = new QSortFilterProxyModel(this);

    ui.tableView->setModel(0);
    delegate = new MyDelegate(ui.tableView);
    ui.tableView->setItemDelegate(delegate);
    connect(delegate,SIGNAL(changeData()),this,SLOT(slotAttManualChange()));
    connect(ui.treeView,SIGNAL(clicked(const QModelIndex &)),this,SLOT(groupChanged(const QModelIndex &)) );

    xlsName=xsdStr;
   setDocument(xmlStr);
   if(schema->getValid())	xlsName=xsdStr;
   else 					xlsName="";
}

deditor::~deditor()
{
	if(tableAttr) delete tableAttr;
	delete(delegate);
	delete(model);
	if(schema) delete schema;
}


bool deditor::xmlValidator(const QString& xsdName, const QString& Name,QDomDocument* doc){

	QXmlSchema sch;
	QDomDocument xsddoc;
	QDomDocument cdoc;
	QDomDocument* ddoc;
	if(doc) ddoc=doc;
	else    ddoc=&cdoc;

    QFile filexsd(xsdName);
    if(!filexsd.open(QIODevice::ReadOnly | QIODevice::Text)){
	     qDebug() <<"Not open file:"+xsdName;
	     return false;
     }else{
		if(!xsddoc.setContent(&filexsd)){
			qDebug() <<"Error parse file:"+xsdName;
			filexsd.close();
			return false;
		}else{
			filexsd.close();
			sch.load(xsddoc.toByteArray());
			if(!sch.isValid()){
				 qDebug() <<"Schema invalid:"+xsdName;
				 return false;
			}
		}
     }

    if(Name=="") return false;

	QFile file(Name);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
		qDebug() <<"Not open file:"+Name;
		return false;
	}

	if(!ddoc->setContent(&file)){
		ddoc->clear();
		file.close();
		qDebug() <<"Error parse file:"+Name;
		return false;
	}
	file.close();

	QXmlSchemaValidator validator(sch);
	if (!validator.validate(ddoc->toByteArray())){
		qDebug() <<"XML document invalid!";
		return false;
	}
	return true;
}

bool deditor::setDocument(const QString& Name){

	if(Name!=""){
		if(!xmlValidator(xlsName,Name,&document)){
			QFile::remove(Name);
			XSD::createFile(Name,".");
			xmlValidator(xlsName,Name,&document);
		}
		xmlName=Name;
		/*if(schema!=NULL) delete(schema);
		schema = new xmlschema(xlsName,&document);
		if(!schema->getValid()) return false;
		rootNode=document.documentElement();
		*/
	}else 	document=QDomDocument();

	//else return false;  //Работаем с дефолтным доком
        if(schema!=NULL) delete(schema);
	schema = new xmlschema(xlsName,&document);
	if(!schema->getValid()) return false;
	rootNode=document.documentElement();

	DataChange=false;


    if(model!=NULL)  delete(model);
	model  = new DomModel(schema,this);
	model->MapIcons=&MapIcons;
	model->MapIconsDis=&MapIconsDis;
	model->MapIconsEn=&MapIconsEn;
	model->MapStatus=&MapIdStatus;
	model->MapMask=&MapMask;
	model->AddMask=&AddMask;
	model->SepMask=&SepMask;
	model->IdAttr=&IdAttr;

	treeProxyModel->setSourceModel(model);
	treeProxyModel->setFilterRole(Qt::DisplayRole);

	treeProxyModel->setFilterKeyColumn(1);
	treeProxyModel->setFilterRegExp(QRegExp("[1,2,3]")); //группы,элементы и пустые элементы
		// ui.treeView->setSelectionMode(QAbstractItemView::SingleSelection);
		// ui.treeView->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui.treeView->setModel(treeProxyModel);
		 //ui.treeView->setModel(model);
	ui.treeView->hideColumn(1); //тип узла
	ui.treeView->hideColumn(2); //текстовое значение узла

	// ui.treeView->hideColumn(4);
	// ui.treeView->expandAll();
	 //ui.treeView->header()->setStretchLastSection(true);
	 // ui.treeView->->setAcceptDrops(true);
	 // ui.treeView->->setDropIndicatorShown(true);

	 //   QColor color = QColor(200, 230, 240);
	  //  treeview->setStyleSheet(QString(
      //  "QTreeView{ background-color: %1 }").arg(color.name()));
	// tableProxyModel = new QSortFilterProxyModel(this);
	// tableProxyModel->setSourceModel(model);
	 //tableProxyModel->setFilterRole(Qt::DisplayRole);
	 //tableProxyModel->setFilterKeyColumn(4);
	//ui.tableView->setModel(tableProxyModel);
	    //ui.tableView->hideColumn(3);
	   // ui.tableView->hideColumn(4);
	   // ui.tableView->resizeRowsToContents();
	   // ui.tableView->resizeColumnsToContents();
	   // ui.tableView->horizontalHeader()->setStretchLastSection(true);
	    // ui.tableView->setSelectionMode(QAbstractItemView::SingleSelection);
	    // ui.tableView->setDragEnabled(true); ///////
	    // ui.tableView->setDropIndicatorShown(true);

	//переход на первый элемент и его открытие
	QModelIndex pit=indexTreeFromNode(rootNode);
	ui.treeView->setCurrentIndex(pit);
	groupChanged(pit);
	ui.treeView->setExpanded(pit,true);

    return true;
}


/*
 * Смена группы в дереве с переинициализацией редактора атрибутов
 */

void deditor::groupChanged(const QModelIndex& index){
    QModelIndex si = treeProxyModel->mapToSource(index);

    DomAttr* DA=NULL;
    DomItem* DI=NULL;
    if(si.isValid()){
    	if(model->nodeFromIndex(si)->node().toElement()!=schema->rootxml){
    		DI=model->nodeFromIndex(si);
     	}
   		emit changeCurNode(model->nodeFromIndex(si)->node());
    }

    DA=new DomAttr(DI,schema,this);
    DA->setColorList(noEdColor,AllColored,ColoredList.join(","));
    ui.tableView->setModel(DA);

    if(tableAttr!=NULL) delete tableAttr;
   	tableAttr=DA;

    ui.tableView->resizeRowsToContents();
    ui.tableView->resizeColumnsToContents();
    ui.tableView->horizontalHeader()->setStretchLastSection(true);

}



/*
 * Запись результатов редактирования
 */
bool deditor::saveCurDom(){

	if(!model) return false;
	if(xmlName=="") return false;
	QFile file(xmlName);
	if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
		qDebug() <<"Not open write file:"+xmlName;
		return false;
	}
	QTextStream out(&file);
	out.setCodec("UTF-8");
	out<<document.toString();
	file.close();
	DataChange=false;
	return true;
}

/*
 * Запись результатов редактирования
 */
void deditor::saveDom(){
	if(saveCurDom()) emit isSave();
}
/*
 * Запись по имени
 */
bool deditor::saveDomAs(const QString& Name){
	if(!model)   return false;
	if(Name=="") return false;
	if(xmlName==Name){
		return saveCurDom();
	}

	QFile file(Name);
	if(!file.open(QIODevice::WriteOnly | QIODevice::Text)){
		qDebug() <<"Not open write file:"+Name;
		return false;
	}
	QTextStream out(&file);
	out.setCodec("UTF-8");
	out<<document.toString();
	file.close();
	if(xmlName=="") xmlName=Name;
	return true;
}

/*
 * Удаление узла дерева
 */
void deditor::deleteNode(const QModelIndex& ci){
	if(!model) return;
	if(!ci.isValid()) return;//элемент выбран

	int rez=isDeleted(ci);
	if(rez==0){
		emit sendMess(tr("You can not delete!"));
		return;
	}
	if(rez<0){
		emit sendMess(tr("You can not delete! Only replace"));
		return;
	}

	int row=ci.row();


    //на родителя и свертка
	QModelIndex pi=model->parent(treeProxyModel->mapToSource(ci));
	QModelIndex pit=treeProxyModel->mapFromSource(pi);
	ui.treeView->setCurrentIndex(pit);
	ui.treeView->setExpanded(pit,false);

	QModelIndex d;
	QString id;
	//----------------------------
    if(rez==1){
    	d=model->index(row, 0, pi);
    	deleteNodeModel(d);
    }else{
		for(int i=rez-1;i>=0;i--){
			d=model->index(i, 0, pi);
			deleteNodeModel(d);
		}
    }


	//разворачиваем и встаем на предшественника
	ui.treeView->setExpanded(pit,true);
	groupChanged(ui.treeView->currentIndex());
    //------------------------------------------
	emit sendMess(tr("The nodes are removed"));
	DataChange=true;
	emit changeData();
	if(deletedId.size()){
		emit deleteNode(deletedId);
		deletedId.clear();
	}


}

/*
 *   ищем по id в дереве
 */
QModelIndex deditor::searchIDTreeIndex(const QString& id){
	if(IdAttr=="" || id=="") return QModelIndex();
	QDomElement el=searchNodeID(rootNode,id);
	if(el.isNull()) return QModelIndex();
	return indexTreeFromNode(el);
}

/*
 *   ищем по id рекурсивно
 */
QDomElement deditor::searchNodeID(QDomElement el,const QString& id){
	if(el.hasAttribute(IdAttr)){
		if(el.attribute(IdAttr)==id) return el;
	}else{
		QDomNodeList ln=el.childNodes();
		for(int i=0;i<ln.size();i++){
			QDomElement cur=searchNodeID(ln.at(i).toElement(),id);
			if(!cur.isNull()) return cur;
		}
	}
	return QDomElement();
}

/*
 * Изменение статуса элемента с id
 */
void deditor::setIdStatus(QString id,int status){
	//qDebug()<<id;
	if(status!=0) MapIdStatus[id]=status;
	else if(MapIdStatus.contains(id)) MapIdStatus.remove(id);
	QModelIndex index=searchIDTreeIndex(id);
	if(index.isValid()){
		model->changeDataM(treeProxyModel->mapToSource(index));
	}
}

/*
 * Сброс статусов всех
 */
void deditor::clearIdSatus(){
	MapIdStatus.clear();
	QModelIndex is=indexTreeFromNode(rootNode);
	ui.treeView->collapse(is);
	ui.treeView->expand(is);
}

/*
 * Переход в дереве на родителя
 */
void deditor::goParent(const QModelIndex& ci){
	if(!model) return;
	if(!ci.isValid()) return;//элемент выбран

	if(model->nodeFromIndex(treeProxyModel->mapToSource(ci))->node().toElement()==schema->rootxml.toElement()){
		return;
	}

    //на родителя и свертка
	QModelIndex pi=model->parent(treeProxyModel->mapToSource(ci));
	QModelIndex pit=treeProxyModel->mapFromSource(pi);
	ui.treeView->setCurrentIndex(pit);
	groupChanged(ui.treeView->currentIndex());
    //------------------------------------------
}

/*
 * Переход в дереве на элемент
 */
void deditor::goElement(const QDomElement element){

	if(!model || element.isNull()) return;
	QModelIndex ci=indexTreeFromNode(element);
	if(!ci.isValid()) return;
	ui.treeView->setCurrentIndex(ci);
	groupChanged(ui.treeView->currentIndex());
    //------------------------------------------
}


/*
 * Рекурсивное удаление узла по его индексу -
 */
bool deditor::deleteNodeModel(const QModelIndex& si){

	if(IdAttr=="") return model->removeRows(si.row(), 1, model->parent(si));

	int cou=model->rowCount(si);//дочерние узлы
	for(int i=cou-1;i>=0;i--) deleteNodeModel(model->index(i,0,si));
	QString id=model->nodeFromIndex(si)->node().toElement().attribute(IdAttr,"");
	if(id!="") deletedId<<id;
	return model->removeRows(si.row(), 1, model->parent(si));
}

/*
 * Очистить содержимое узла до минимально возможного
 */
void deditor::clearNode(const QModelIndex& cui){
	if(!model) return;
	int cd=0;//было удалено
	if(!cui.isValid()) return;//есть
    QModelIndex si=treeProxyModel->mapToSource(cui);
    ui.treeView->setExpanded(cui,false);

	//назад по описанию
	int cou=model->rowCount(si);
	for(int i=cou-1;i>=0;i--){
		QModelIndex ci=model->index(i,0,si);
		if(isDeleted(model->nodeFromIndex(ci)->node().toElement())>0){
			deleteNodeModel(ci);
			cd++;
		}
	}

	//на элемент
	ui.treeView->setCurrentIndex(cui);
	ui.treeView->setExpanded(cui,true);
	groupChanged(cui);

	QString txt;
	emit sendMess(tr("Removed nodes: ")+txt.setNum(cd));
	DataChange=true;
	emit changeData();
	if(deletedId.size()){
		emit deleteNode(deletedId);
		deletedId.clear();
	}
}


/*
 * Добавить конкретный узел
 */
QDomElement deditor::addNode(const QModelIndex& ci,const QString& name){
	QDomElement nEl;
	if(!model) return nEl;
	if(!ci.isValid()) return nEl;//есть
	int cd=0;//было добавлено

	QModelIndex si=treeProxyModel->mapToSource(ci);
	DomItem* dit=model->nodeFromIndex(si);
	QDomElement de=dit->node().toElement();
	if(de.isNull() || schema==NULL) return nEl;

	QDomNodeList lc=de.childNodes();//список фактических потомков родителя
	NodeRatio   Ratio=schema->getXLSTypeRatioCild(de);
	QVector<QDomNode> no=schema->getChild(de);

	//ищем описание потомка по имени разрешенном списке
	if(!addNodeList(ci).contains(name)){
		 sendMess(tr("Invalid node"));
		 return nEl;
	}

	int index=-1;     //позиция в списке описаний
	QDomNode addn; //само описание
	for(int i=0; i<no.count();i++){
		if(schema->getNameXLS(no.at(i))==name){
			addn=no.at(i);
			index=i;
			break;
		}
	}
	//добавляем в соотв место

     int n=0; //позиция добавления
     QString sname;
     QString tt;
     //нужно вычислить позицию вставки по описанию
	 switch(Ratio){
		case all: //в конец
			n=lc.count();
			break;
		case sequence: //вставка
			for(int i=index; i>=0;i--){//перебираем описания элементов начиная с самого и выше
				sname=schema->getNameXLS(no.at(i));
				//ищем с конца
				for(int i=lc.count()-1;i>=0;i--){
					if(lc.at(i).nodeName()==sname){
						n=i+1;
						break;
					}
				}
				if(n>0) break;
			}
			break;
		case choice: //замена
			n=0;
			break;
	}

	int k=schema->getMinOccursXLS(addn);//кратность
	if(k==0) k=1;

	ui.treeView->setExpanded(ci,false);

	nEl=document.createElement(name);
	for(int i=n;i<n+k;i++){
		model->setNewEl(nEl);
		if(model->insertRows(i,1,si,getAttr)){
			addNodeModel(model->getNewIn());//рекурсивно потомки.
			cd++;
		}else	 emit sendMess(tr("You can no longer add items!"));
		if(i<n+k-1) nEl=nEl.cloneNode().toElement();
	}

	QString txt;
	ui.treeView->setExpanded(ci,true);
	emit sendMess(tr("Posted nodes: ")+txt.setNum(cd));
	if(manualChange) {
		manualChange=false;
		emit AddManualNode(nEl);
	}
	DataChange=true;
	emit changeData();
	return nEl;
}

/*
 * Добавить атрибут
 */
void deditor::addAttribute(const QModelIndex& ci,const QString& name,const QString& v){

	if(!model) return;
	if(!ci.isValid()) return;//есть

	QModelIndex si=treeProxyModel->mapToSource(ci);
	DomItem* dit=model->nodeFromIndex(si);
	QDomElement de=dit->node().toElement();
	if(de.isNull() || schema==NULL) return;

	QStringList AddList=addAttributeList(ci);
	if(!AddList.contains(name)) return;
	//конец проверок

	//описания всех атрибутов
	QVector<QDomNode> allAtt= schema->getAttributes(de,getAll);
	//описание нашего
	QDomNode opAttr;
	for(int i=0;i<allAtt.size();i++){
		if(allAtt.at(i).attributes().namedItem("name").nodeValue()==name){
			opAttr=allAtt.at(i);
			break;
		}
	}

	if(v.size()==0) de.setAttribute (name,schema->getAttributeXLSDefault(opAttr));
	else            de.setAttribute (name,v);
	groupChanged(ci); //освежить таблицу
}

/*
 * Удалить атрибут
 */
void deditor::deleteAttribute(const QModelIndex& ci,const QString& name){

	if(!model) return;
	if(!ci.isValid()) return;//есть

	QModelIndex si=treeProxyModel->mapToSource(ci);
	DomItem* dit=model->nodeFromIndex(si);
	QDomElement de=dit->node().toElement();
	if(de.isNull() || schema==NULL) return;

	QStringList DeleteList=clearAttributeList(ci);
	if(!DeleteList.contains(name)) return;
	//конец проверок

	de.removeAttribute(name);
	groupChanged(ci); //освежить таблицу
}

/*
 * Добавление для текущего элемента всех обязательных потомков рекурсивно
 */

bool deditor::addNodeModel(const QModelIndex& si){


	DomItem* dit=model->nodeFromIndex(si);
	QDomElement de=dit->node().toElement();
	if(de.isNull() || schema==NULL) return false;
	int cCou,mCou,cou=0;

	NodeRatio Ratio=schema->getXLSTypeRatioCild(de);
	QVector<QDomNode> nl=schema->getChild(de);
	if(nl.count()==0) return false;

	//для отношения типа choice ищем нулевые элементы - если есть ничего не вставляем
	if(Ratio==choice){
		for(int i=0;i<nl.count();i++){
				if(schema->getMinOccursXLS(nl.at(i))==0) return false;
		}
	}

	for(int i=0;i<nl.count();i++){
		cou	=schema->getMinOccursXLS(nl.at(i));//должно быть не меньше
		mCou=schema->getMaxOccursXLS(nl.at(i));//не больше
		//if(addOpt && cou==0) cou=1;
		if(mCou>0){//есть ограничение на максимум
			//сколько такие уже есть
			cCou=countChildElement(de,schema->getNameXLS(nl.at(i)));
			if(cCou>=mCou) continue; //больше добавлять нельзя
		}

		for(int k=0;k<cou;k++){
			//определяем элемент для вставки
			QDomElement nEl=document.createElement(schema->getNameXLS(nl.at(i)));
			model->setNewEl(nEl);
			//куда вставить
			if(model->insertRows(model->rowCount(si),1,si,getAttr)){
				addNodeModel(model->getNewIn());//рекурсивно потомки.
			}
		}
		if(Ratio==choice && cou>0) return true;//что-то вставили - больше нельзя
	}
	return true;
}

/*
 * список атрибутов для добавления
 */
QStringList deditor::addAttributeList(const QModelIndex& ci){
	QStringList rez;
	if(!ci.isValid()) return rez;
	QDomElement de=model->nodeFromIndex(treeProxyModel->mapToSource(ci))->node().toElement();
	if(de.isNull() || schema==NULL) return rez;
	QStringList allAtt= schema->getAttributesName(de,getAll);//getOptional
	QDomNamedNodeMap curAtt=de.attributes();
	for(int i=0; i<allAtt.size();i++){
		if(!curAtt.contains(allAtt.at(i))){
			rez<<allAtt.at(i);
		}
	}
	return rez;
}

/*
 * список атрибутов для удаления
 */
QStringList deditor::clearAttributeList(const QModelIndex& ci){
	QStringList rez;
	if(!ci.isValid()) return rez;
	QDomElement de=model->nodeFromIndex(treeProxyModel->mapToSource(ci))->node().toElement();
	if(de.isNull() || schema==NULL) return rez;
	QStringList allAttOpt= schema->getAttributesName(de,getOtional);
	QDomNamedNodeMap curAtt=de.attributes();
	for(int i=0; i<allAttOpt.size();i++){
		if(curAtt.contains(allAttOpt.at(i))){
			rez<<allAttOpt.at(i);
		}
	}
	return rez;
}

/*
 * список для добавления
 */
QStringList deditor::addNodeList(const QModelIndex& ci){
	QStringList rez;

	if(!ci.isValid()) return rez;
	QDomElement de=model->nodeFromIndex(treeProxyModel->mapToSource(ci))->node().toElement();
	if(de.isNull() || schema==NULL) return rez;
	rez= schema->getAddChildName(de);
	return rez;
}

QStringList deditor::insertNodeBeforeList(const QModelIndex& ci){
	QStringList rez;

	if(!ci.isValid()) return rez;
	QDomElement de=model->nodeFromIndex(treeProxyModel->mapToSource(ci))->node().toElement();
	if(de.isNull() || schema==NULL) return rez;
	rez= schema->getInsBeforeElementsName(de);
	return rez;
}

QStringList deditor::insertNodeAfterList(const QModelIndex& ci){
	QStringList rez;

	if(!ci.isValid()) return rez;
	QDomElement de=model->nodeFromIndex(treeProxyModel->mapToSource(ci))->node().toElement();
	if(de.isNull() || schema==NULL) return rez;
	rez= schema->getInsAfterElementsName(de);
	return rez;
}

QStringList deditor::replaceNodeList(const QModelIndex& ci){
	QStringList rez;

	if(!ci.isValid()) return rez;
	QDomElement de=model->nodeFromIndex(treeProxyModel->mapToSource(ci))->node().toElement();
	if(de.isNull() || schema==NULL) return rez;
	rez= schema->getReplaiceName(de);
	return rez;
}

/*
 * Можно-ли удалить элемент?
 * 0     -нельзя
 * 1     -можно
 * N>1   -только всей группой  в количестве N
 * -1    -только с заменой
 * -N<-1  -всю группу с заменой
 */
int deditor::isDeleted(const QModelIndex& ci){
	if(!ci.isValid()) return false;
	QDomElement de=model->nodeFromIndex(treeProxyModel->mapToSource(ci))->node().toElement();
	return isDeleted(de);
}
/*
 * Можно-ли удалить элемент? - по его имени в DOM
 * 0     -нельзя
 * 1     -можно
 * N>1   -только всей группой  в количестве N
 * -1    -только с заменой
 * -N<-1  -всю группу с заменой
 */
int deditor::isDeleted(const QDomElement de){
	if(de.isNull())  return 0;
	if(schema==NULL) return 1;
	if(de==schema->rootxml.toElement()) return 0;
	NodeRatio Ratio=schema->getXLSTypeRatio(de); //отношение внутри группы

	int nmin=schema->getMinOccursXLS(de);//минимум
	int cou=countChildElement(de.parentNode().toElement(),de.tagName ()); //по факту

	if(Ratio!=choice){
		if(cou>nmin) return 1;
	}else{
		//можно удалить все? -есть элемент с нулевым количеством
		QVector<QDomNode> xlsc=schema->getChild(de.parentNode().toElement());
		bool clCh=false;
		for(int i=0;i<xlsc.count();i++){
			if(schema->getMinOccursXLS(xlsc.at(i))==0){// нет ограничения с низу хотя-бы у одного
				 clCh=true;
				 break;
			}
		}
		if(cou>nmin)  return 1;     //можно один
		else if(clCh) return cou;   //только все сразу
		else          return -1*cou;//только с заменой
	}
    return 0;
}

/*
 * Список имен узлов, которые можно удалить из родителя
 */
QStringList deditor::clearNodeList(const QModelIndex& ci){
	QStringList rez;

	if(!ci.isValid()) return rez;
	QDomElement de=model->nodeFromIndex(treeProxyModel->mapToSource(ci))->node().toElement();
	if(de.isNull() || schema==NULL) return rez;
	NodeRatio Ratio=schema->getXLSTypeRatioCild(de); //отношение внутри типа
	QVector<QDomNode> child= schema->getChild(de);
	bool clCh=false;
	if(Ratio==choice){
		for(int i=0;i<child.count();i++){
			if(schema->getMinOccursXLS(child.at(i))==0){// нет ограничения с низу хотя-бы у одного
				 clCh=true;
				 break;
			}
		}
	}

	for(int i=0; i<child.count();i++){
		//по факту
		int cou=countChildElement(de,schema->getNameXLS(child.at(i))); //по факту
		int min=schema->getMinOccursXLS(child.at(i));
		if(cou>min || clCh) rez<<schema->getNameXLS(child.at(i));
	}

	return rez;
}



void deditor::addEditToContextMenu(QMenu& MyMenu,QModelIndex ci,int mask=255){

	if(!ci.isValid()) return;

	if(mask & AddNode){
		//определяемся с добавлением узлов из родителя
		QList<QAction*>   listAddAct;
		QStringList child= addNodeList(ci);
		if(MapperAdd!=NULL) delete(MapperAdd);
		MapperAdd= new QSignalMapper(this);
		for(int i=0; i<child.count();i++){
			listAddAct<<new QAction(child.at(i),this);
			connect(listAddAct.at(i), SIGNAL(triggered()),MapperAdd,SLOT(map()));
			MapperAdd->setMapping(listAddAct.at(i),child.at(i));
		}
		if(listAddAct.count()>0 && MapperAdd!=NULL){
			QMenu* SubMenuAdd = new QMenu(tr("Add node"),&MyMenu);
			for(int i=0;i<listAddAct.count();i++) SubMenuAdd->addAction(listAddAct.at(i));
			MyMenu.addMenu(SubMenuAdd);
			connect(MapperAdd,SIGNAL(mapped(const QString&)),this,SLOT(addNode(const QString&)));
		}
	}
	//------------------------------------------------------------------------------------------------

	if(mask & AddBefore){
		//определяемся со вставкой до   insert before
		QList<QAction*>   listBeforeAct;
		QStringList badd=insertNodeBeforeList(ci);
		if(MapperBefore!=NULL) delete(MapperBefore);
		MapperBefore= new QSignalMapper(this);
		for(int i=0; i<badd.count();i++){
			listBeforeAct<<new QAction(badd.at(i),this);
			connect(listBeforeAct.at(i), SIGNAL(triggered()),MapperBefore,SLOT(map()));
			MapperBefore->setMapping(listBeforeAct.at(i),badd.at(i));
		}
		if(listBeforeAct.count()>0 && MapperBefore!=NULL){
			QMenu* SubMenuAddBefore = new QMenu(tr("Add node before"),&MyMenu);
			for(int i=0;i<listBeforeAct.count();i++) SubMenuAddBefore->addAction(listBeforeAct.at(i));
			MyMenu.addMenu(SubMenuAddBefore);
			connect(MapperBefore,SIGNAL(mapped(const QString&)),this,SLOT(insertNodeBefore(const QString&)));
		}
	}
	//----------------------------------------------------------------------------------------

	if(mask & AddAfter){
		//определяемся со вставкий после insert after
		QList<QAction*>   listAfterAct;
		QStringList aadd=insertNodeAfterList(ci);
		if(MapperAfter!=NULL) delete(MapperAfter);
		MapperAfter= new QSignalMapper(this);
		for(int i=0; i<aadd.count();i++){
			listAfterAct<<new QAction(aadd.at(i),this);
			connect(listAfterAct.at(i), SIGNAL(triggered()),MapperAfter,SLOT(map()));
			MapperAfter->setMapping(listAfterAct.at(i),aadd.at(i));
		}
		if(listAfterAct.count()>0 && MapperAfter!=NULL){
			QMenu* SubMenuAddAfter = new QMenu(tr("Add node after"),&MyMenu);
			for(int i=0;i<listAfterAct.count();i++) SubMenuAddAfter->addAction(listAfterAct.at(i));
			MyMenu.addMenu(SubMenuAddAfter);
			connect(MapperAfter,SIGNAL(mapped(const QString&)),this,SLOT(insertNodeAfter(const QString&)));
		}
	}
	//-------------------------------------------------------------------------------------------

	if(mask & ReplaceNode){
		//определяемся с заменой узлов
		QList<QAction*>   listReplaceAct;
		QStringList rchild=replaceNodeList(ci);
		if(MapperReplace!=NULL) delete(MapperReplace);
		MapperReplace= new QSignalMapper(this);
		for(int i=0; i<rchild.count();i++){
			listReplaceAct<<new QAction(rchild.at(i),this);
			connect(listReplaceAct.at(i), SIGNAL(triggered()),MapperReplace,SLOT(map()));
			MapperReplace->setMapping(listReplaceAct.at(i),rchild.at(i));
		}
		if(listReplaceAct.count()>0 && MapperReplace!=NULL){
			QMenu* SubMenuReplace = new QMenu(tr("Replace node"),&MyMenu);
			for(int i=0;i<listReplaceAct.count();i++) SubMenuReplace->addAction(listReplaceAct.at(i));
			MyMenu.addMenu(SubMenuReplace);
			connect(MapperReplace,SIGNAL(mapped(const QString&)),this,SLOT(replaceNode(const QString&)));
		}
	}
	//-----------------------------------------------------------------------------------------------

	if(mask & DeleteNode){
		//определяемся с удалением узла
		if(isDeleted(ci)>0 || schema==NULL) MyMenu.addAction(delAct);
	}
	//--------------------------------------------------
	if(mask & ClearNode){
		//определяемся с очисткой - конечный/пустой не чистим
		if(clearNodeList(ci).count()>0 || schema==NULL){
			MyMenu.addAction(clearAct);
		}
	}

	if(mask & AddAttribute){
		//определяемся с атрибутами для добавления
		QStringList addAtt=addAttributeList(ci);
		QList<QAction*>   listAddAttAct;
		if(MapperAddAtt!=NULL) delete(MapperAddAtt);
		MapperAddAtt= new QSignalMapper(this);
		for(int i=0;i<addAtt.count();i++){
			listAddAttAct<<new QAction(addAtt.at(i),this);
			connect(listAddAttAct.at(i), SIGNAL(triggered()),MapperAddAtt,SLOT(map()));
			MapperAddAtt->setMapping(listAddAttAct.at(i),addAtt.at(i));

		}
		if(addAtt.count()>0 && MapperAddAtt!=NULL){
			QMenu* SubMenuAddAtt = new QMenu(tr("Add attribute"),&MyMenu);
			for(int i=0;i<addAtt.count();i++) SubMenuAddAtt->addAction(listAddAttAct.at(i));
			MyMenu.addMenu(SubMenuAddAtt);
			connect(MapperAddAtt,SIGNAL(mapped(const QString&)),this,SLOT(addAttribute(const QString&)));
		}
	}

	if(mask & DeleteAttribute){
		//определяемся с атрибутами для удаления
		QStringList delAtt=clearAttributeList(ci);
		QList<QAction*>   listClearAttAct;
		if(MapperDelAtt!=NULL) delete(MapperDelAtt);
		MapperDelAtt= new QSignalMapper(this);
		for(int i=0;i<delAtt.count();i++){
			listClearAttAct<<new QAction(delAtt.at(i),this);
			connect(listClearAttAct.at(i), SIGNAL(triggered()),MapperDelAtt,SLOT(map()));
			MapperDelAtt->setMapping(listClearAttAct.at(i),delAtt.at(i));

		}
		if(delAtt.count()>0 && MapperDelAtt!=NULL){
			QMenu* SubMenuDelAtt = new QMenu(tr("Delete attribute"),&MyMenu);
			for(int i=0;i<delAtt.count();i++) SubMenuDelAtt->addAction(listClearAttAct.at(i));
			MyMenu.addMenu(SubMenuDelAtt);
			connect(MapperDelAtt,SIGNAL(mapped(const QString&)),this,SLOT(deleteAttribute(const QString&)));
		}
	}

}
/*
 * Контекстное меню
 */
void deditor::showContextMenuForTreeView(const QPoint& pos){

	if(!model) return;
	QMenu MyMenu;

	QModelIndex ci=ui.treeView->currentIndex();
	if(!ci.isValid()) return;//элемент выбран
	groupChanged(ci);//встаем на элемент
	QString Name=model->nodeFromIndex(treeProxyModel->mapToSource(ci))->node().nodeName();

	int mask;
	if(EditableFromMenu){
		mask=255;//все
		if(NoEdContextMenu.contains("")) mask&=(~NoEdContextMenu[""]);
		if(NoEdContextMenu.contains(Name)) mask&=(~NoEdContextMenu[Name]);

	}else{
		mask=0;//ни один
		if(EdContextMenu.contains("")) mask|=EdContextMenu[""];
		if(EdContextMenu.contains(Name)) mask|=EdContextMenu[Name];
	}
	addEditToContextMenu(MyMenu,ci,mask);

	//Имя элемента внешние акции
	if(ExtActionTree.contains(Name)){
		for(int i=0;i<ExtActionTree[Name].count();i++){
			if(ExtActionTree[Name].at(i)->isEnabled()){
			   MyMenu.addAction(ExtActionTree[Name].at(i));
			}
		}
	}

	if(!MyMenu.isEmpty()) MyMenu.exec(ui.treeView->viewport()->mapToGlobal(pos));
}

/*
 * Контекстное меню таблицы
 */

void deditor::showContextMenuForTableView(const QPoint& pos){

	if(ExtActionTable.count()==0) return;

	QMenu MyMenu;

	QModelIndex ci=ui.tableView->currentIndex();
	//Имя элемента
	QString Name=tableAttr->DomNodeFromIndex(ci).nodeName();
	//внешние акции
	if(ExtActionTable.contains(Name)){
		for(int i=0;i<ExtActionTable[Name].count();i++){
			MyMenu.addAction(ExtActionTable[Name].at(i));
		}
	}
	if(!MyMenu.isEmpty()) MyMenu.exec(ui.tableView->viewport()->mapToGlobal(pos));
}
/*
 * Замена текущего узла на указанный
 *  на допустимость уже не проверяем?
 */
void deditor::replaceNode(const QModelIndex& ci,const QString& ni){
	if(!model) return;
	if(!ci.isValid()) return;//элемент выбран
	int cd=0;//сколько вставили

	QModelIndex si=treeProxyModel->mapToSource(ci);
	QModelIndex pi=model->parent(si);//папа
	if(!pi.isValid()){
		emit sendMess(tr("You can not remove the root element!"));
		return;// корень - удалять нельзя!!!
	}

	DomItem* dit=model->nodeFromIndex(si);
	QDomNode dn=dit->node();
	QDomElement de=dn.toElement();


	DomItem* pdit=model->nodeFromIndex(pi);
	QDomNode pdn=pdit->node();
	QDomElement pde=pdn.toElement();
	QVector<QDomNode> nl=schema->getChild(pde);
	NodeRatio Ratio=schema->getXLSTypeRatioCild(pde);
    if(Ratio==sequence) return; //нельзя заменять для данного типа отношений
    //проверки!!!
	int cCou=countChildElement(pde,de.tagName()); //сейчас таких элементов у родителя

	int cou;
	if(Ratio==choice) cou=0;
	else              cou=schema->getMinOccursXLS(de);  //должно быть не меньше
	if(cCou<=cou) return;//нельзя удалять


	QModelIndex pit=treeProxyModel->mapFromSource(pi);
    ui.treeView->setCurrentIndex(pit);//ушли на родителя

	int row;//куда потом будем вставлять
	if(Ratio==choice){//вообще очищаем всех детей
		row=0;
		for(int i=cCou-1;i>=0;i--){
			if(!deleteNodeModel(model->index(i, 0, pi))) return;
		}
	}else{
		row=si.row();
		if(!deleteNodeModel(si)) return;
	}

	QDomNode addn; //само описание
	for(int i=0; i<nl.count();i++){
		if(nl.at(i).attributes().namedItem("name").nodeValue()==ni){
			addn=nl.at(i);
			break;
		}
	}
	int k=schema->getMinOccursXLS(addn);//кратность
	if(k==0) k=1;

	QDomElement nEl=document.createElement(ni);
	for(int i=row;i<row+k;i++){
			model->setNewEl(nEl);
			if(model->insertRows(i,1,pi,getAttr)){
				addNodeModel(model->getNewIn());//рекурсивно потомки.
				cd++;
			}else	 emit sendMess(tr("You can no longer add items!"));
			if(i<row+k-1) nEl=nEl.cloneNode().toElement();
	}

	groupChanged(ui.treeView->currentIndex());
	if(manualChange) {
		manualChange=false;
		emit AddManualNode(nEl);
	}
	if(cd>0){
		emit sendMess(tr("The nodes are replaced!"));
		DataChange=true;
		emit changeData();
	}
}

void deditor::insertNodeBefore(const QModelIndex& ci,const QString& ni){
	if(!model) return;
	if(!ci.isValid()) return;//элемент выбран
	bool Chenge=false;
	QModelIndex si=treeProxyModel->mapToSource(ci);
	DomItem* dit=model->nodeFromIndex(si);
	QDomNode dn=dit->node();
	QDomElement de=dn.toElement();
	if(de==schema->rootxml) return;//корень
	QModelIndex pi=model->parent(si);//папа
	int row=si.row();

	//по идее надо проверить допустимость имени...
	if(!insertNodeBeforeList(ci).contains(ni)){
		emit sendMess(tr("Invalid node!"));
		return;
	}

	QDomElement nEl=document.createElement(ni);
	model->setNewEl(nEl);

	//на родителя и свертка
	int rr=ci.row();
	QModelIndex pit=treeProxyModel->mapFromSource(pi);
	ui.treeView->setCurrentIndex(pit);
	ui.treeView->setExpanded(pit,false);
	//-------------------------

	if(model->insertRows(row,1,pi,getAttr)){
			addNodeModel(model->getNewIn());//рекурсивно потомки.
			Chenge=true;
			rr++;
	}else	emit sendMess(tr("You can no longer add items!"));

	//разворачиваем на место
	ui.treeView->setExpanded(pit,true);
	ui.treeView->setCurrentIndex(treeProxyModel->index(rr,0,pit));
	groupChanged(ui.treeView->currentIndex());
	if(manualChange) {
		manualChange=false;
		emit AddManualNode(nEl);
	}
	if(Chenge){
		emit sendMess(tr("The nodes are added!"));
		DataChange=true;
		emit changeData();
	}
}

void deditor::insertNodeAfter(const QModelIndex& ci, const QString& ni){
	if(!model) return;
	if(!ci.isValid()) return;//элемент выбран
	bool Chenge=false;
	QModelIndex si=treeProxyModel->mapToSource(ci);

	DomItem* dit=model->nodeFromIndex(si);
	QDomNode dn=dit->node();
	QDomElement de=dn.toElement();
	if(de==schema->rootxml) return;//корень

	QModelIndex pi=model->parent(si);//папа
	int row=si.row();

	//по идее надо проверить допустимость имени...
	if(!insertNodeAfterList(ci).contains(ni)){
		emit sendMess(tr("Invalid node!"));
		return;
	}

	QDomElement nEl=document.createElement(ni);
	model->setNewEl(nEl);

	//на родителя и свертка
	int rr=ci.row();
	QModelIndex pit=treeProxyModel->mapFromSource(pi);
	ui.treeView->setCurrentIndex(pit);
	ui.treeView->setExpanded(pit,false);
		//-------------------------
	if(model->insertRows(row+1,1,pi,getAttr)){
			addNodeModel(model->getNewIn());//рекурсивно потомки.
			Chenge=true;
	}else	emit sendMess(tr("You can no longer add items!"));

	//разворачиваем на место
	ui.treeView->setExpanded(pit,true);
	ui.treeView->setCurrentIndex(treeProxyModel->index(rr,0,pit));

	if(manualChange) {
		manualChange=false;
		emit AddManualNode(nEl);
	}
	//groupChanged(ui.treeView->currentIndex());
	if(Chenge){
		emit sendMess(tr("The nodes are added!"));
		DataChange=true;
		emit changeData();
	}
}

/*
 * Возвращает индекс в модели по заданному элементу документа
 */

QModelIndex deditor::indexFromNode(QDomElement el){

	if(el.isNull()) return QModelIndex();
	//ищем индкс корня
	QModelIndex ci=treeProxyModel->mapToSource(ui.treeView->currentIndex());
	while(model->parent(ci)!=QModelIndex()) ci=model->parent(ci);

	//получаем список всех родителей до рута
	QList<QDomNode> listNode;
	listNode<<el;
	if(el!=schema->rootxml.toElement()){
		QDomNode cur=el.parentNode();
		while(cur!=schema->rootxml){
			listNode.prepend(cur);
			cur=cur.parentNode();
		}
	}
	//теперь ищем наш елемент сверху-вниз
	for(int i=0;i<listNode.count();i++){
		int j=model->nodeFromIndex(ci)->node().childNodes().count();
		for(int row=0;row<j;row++){
			if(listNode.at(i)==model->nodeFromIndex(ci)->child(row)->node()){
				ci=model->index(row,0,ci);
				break;
			}
		}
	}
	return ci;
}

/*
 * Индекс в дереве по элементу
 */
QModelIndex deditor::indexTreeFromNode(QDomElement el){
	return treeProxyModel->mapFromSource(indexFromNode(el));
}


/*
 * Считаем детей по имени не рекурсивно!
 */
int deditor::countChildElement(const QDomElement el,const QString& str){
	int rez=0;
	QDomNodeList list=el.childNodes();
	for(int i=0;i<list.count();i++){
		if(str==list.at(i).nodeName()) rez++;
	}
	return rez;
}

//запретить/разрешить пункты редактирования в меню
void deditor::setEditableFromMenu(bool ed){
	EditableFromMenu=ed;
}

void deditor::setPolicyGetAttribute(bool pAttr){
	getAttr=(getTypeAttr)pAttr;
}

//Слоты для интерактивного редактирования
void deditor::deleteNode(){
	deleteNode(ui.treeView->currentIndex());
}
void deditor::clearNode(){
	clearNode(ui.treeView->currentIndex());
}
void deditor::addNode(const QString& str){
	 manualChange=true;
	 addNode(ui.treeView->currentIndex(),str);
}
void deditor::replaceNode(const QString& str){
	manualChange=true;
	replaceNode(ui.treeView->currentIndex(),str);
}
void deditor::insertNodeBefore(const QString& str){
	manualChange=true;
	insertNodeBefore(ui.treeView->currentIndex(),str);
}
void deditor::insertNodeAfter(const QString& str){
	manualChange=true;
	insertNodeAfter(ui.treeView->currentIndex(),str);
}
void deditor::addAttribute(const QString& str){
	addAttribute(ui.treeView->currentIndex(),str);
}
void deditor::deleteAttribute(const QString& str){
	deleteAttribute(ui.treeView->currentIndex(),str);
}

/*
 * Получить выделенный узел
 */
QDomNode deditor::getCurDomNode(void){
	QModelIndex ci=ui.treeView->currentIndex();
	QDomNode    node=model->nodeFromIndex(treeProxyModel->mapToSource(ci))->node();
	return node;
}

void deditor::setMask(QString key,QStringList m){
	MapMask[key]=m;
}

void deditor::setMask(QString key,QString s){
	setMask(key,s.split(","));
}

void deditor::addMask(QString key,QStringList al,QString r){
	 AddMask[key]=al;
	 SepMask[key]=r;
}
void deditor::addMask(QString key,QString as,QString r){
	 addMask(key,as.split(","),r);
}


void deditor::slotAttManualChange(){
	 DataChange=true;
	 model->changeDataM(indexFromNode(getCurDomNode().toElement()));
	 emit AttManualChange(tableAttr->getParentItem()->node());
	 emit sendMess("Attribute manually changed.");
	 emit changeData();

}

bool deditor::getChangeData() {
	return DataChange;
}

void deditor::setEditableAllAttribute(bool ed){
	delegate->setEditableAll(ed);
	AllColored=!ed;
}

void deditor::setEdAttribute(QString str){
	delegate->setEdList(str);
	AllColored=true;
	ColoredList=str.split(",");
}
void deditor::setNoEdAttribute(QString str){
	delegate->setNoEdList(str);
	AllColored=false;
	ColoredList=str.split(",");
}
void deditor::addEdAttribute(QString str){
	delegate->addEdList(str);
	AllColored=true;
	ColoredList<<str;
}
void deditor::addNoEdAttribute(QString str){
	delegate->addNoEdList(str);
	AllColored=false;
	ColoredList<<str;
}

void deditor::setIcons(QString name,QString file, QString fileDis,QString fileEn){
	if(file.size()) MapIcons[name]=QIcon(file);
	else     if(MapIcons.contains(name)) MapIcons.remove(name);
	if(fileDis.size()) MapIconsDis[name]=QIcon(fileDis);
	else     if(MapIconsDis.contains(name)) MapIconsDis.remove(name);
	if(fileEn.size()) MapIconsEn[name]=QIcon(fileEn);
	else     if(MapIconsEn.contains(name)) MapIconsEn.remove(name);
}

QString deditor::getNameXml(){return xmlName;}
QString deditor::getNameXls(){return xlsName;}

