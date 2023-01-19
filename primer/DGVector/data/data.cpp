#include "data.h"
#include <QtGui/QLabel>
#include <QCloseEvent>

emiterD*  dataC::Emit = 0;

dataC::dataC(QString registers, QWidget *parent) : QDialog(parent)
{
	ui=new Ui::dataClass;
	ui->setupUi(this);
	ui->tableView->setModel(0);
	ui->tableView->resizeRowsToContents();
	ui->tableView->resizeColumnsToContents();
	ui->tableView->horizontalHeader()->setStretchLastSection(true);
	period=500;
	DT=NULL;

	QFile file(registers);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text)){
		qDebug() <<"Not open file:"<<registers;
		return;
	}
	if(!document.setContent(&file)){
			file.close();
			qDebug() <<"Error parse file:"<<registers;
			return;
	}else{
			file.close();
			rootNode=document.documentElement();
			fileName=registers;
	}
	connect(getEmit(),SIGNAL(send(quint32, const QByteArray&)),this,SLOT(updateDat(quint32,const QByteArray&)));
	connect(getEmit(),SIGNAL(sendBAD(quint32)),this,SLOT(badDat(quint32)));
	connect(this,SIGNAL(Close()),this,SLOT(Stop()));
}

dataC::~dataC()
{
	exe.Stop();
	delete(ui);
}

bool dataC::setSrcData(QDomNode snode, uint per,int nu){

	QString name; //полное имя
	QString type; //тип
	QString num;  //номер
	QDomNode parent,src;
	QString port,bitrate,ip,modbus,master;

	int lev=0;  //уровень чтения
	int offset;
	QDomNode		 node;
	QList<QDomNode>  lNode;
	QList<int>       lOffset;
	QList<QDomNode>	 srcL;
	QStringList      NameL;

	bool ok;

	period=per;

	exe.Stop();
	list.clear();
	divider.clear();

	//перечитаем описание
	QFile file(fileName);
	if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) return false;
	if(!document.setContent(&file)){file.close(); return false;}
	else	                                       file.close();


	//тип узла
	if(snode.nodeName().toUpper()=="NODE"){
		parent=snode;
		QDomNodeList cl=snode.childNodes();
		for(int i=0;i<cl.size();i++){
			lNode<<cl.at(i);
		}
		lev=2;
	}else{
		parent=snode.parentNode();//описание канала
		lNode.append(snode);
		lev=1;
	}

	if(parent.isNull() || lNode.size()==0) return false;

	QString id=parent.toElement().attribute("id");
	QString alias=parent.toElement().attribute("alias");
	QString title;
	if(alias!="") 	title=id+" ("+alias+")";
	else 			title=id;



	QString Name;

	for(int j=0;j<lNode.size();j++){

		node=lNode.at(j);
		name=node.nodeName();

		if(node.attributes().contains("type"))	type=node.toElement().attribute("type");
		if(name.left(3).toUpper()=="TAH") type="T";
		num=name.right(1);


		if(document.elementsByTagName(type).count()>0){
			src=document.elementsByTagName(type).at(0);
		}
		srcL<<src;
		Name=node.toElement().attribute("alias");
		if(Name.trimmed()=="") Name=node.toElement().attribute("name");
		if(Name.trimmed()=="") Name=type+num;
		else                   Name=type+num+" ("+Name+")";

		NameL<<Name;
		/*
		for(int i=NameL.size()-2;i>=0;i--){
			if(NameL.at(i)=="") continue;
			if(NameL.at(i)==Name){
				NameL.replace(NameL.size()-1,"");
				break;
			}
		}
		*/


		offset=src.toElement().attributeNode("s"+num).value().toInt(&ok,16);
		QDomNodeList op=src.childNodes();
		for(int i=0;i<op.count();i++){
			if(op.at(i).toElement().attributeNode("read").value().toInt(&ok,10)>=lev){
				list<<op.at(i);
				lOffset<<offset;
				if(op.at(i).attributes().contains("divider")){
						divider<<op.at(i).toElement().attributeNode("divider").value().toInt(&ok,10);
				}else 	divider<<1;
			}
		}
	}

	if(lev==1) title+=" "+Name;
	setWindowTitle(title);

	//!!!!!!!!!!!!!!!!!!!!!!!!!11
	if(DT!=NULL) delete(DT);
	DT=new DataTable(srcL,NameL,this);
	ui->tableView->setModel(DT);
	ui->tableView->setColumnHidden(0,(srcL.size()==1));
	ui->tableView->setColumnWidth(0,100);
	ui->tableView->setColumnWidth(1,50);
	ui->tableView->setColumnWidth(2,60);
	ui->tableView->setColumnWidth(3,60);


	port=parent.toElement().attribute("port","");
	bitrate=parent.toElement().attribute("bitrate");
	master=parent.toElement().attribute("master");
	ip=parent.toElement().attribute("ip","");

	modbus=parent.toElement().attribute("modbus");


	//------------------------------------------------------------
	CountReg=0;
	exe.setTape(); //готовим шарманку
	//exe.debug=true;
   QString txt;


		dIF* di;
		dCONNECT*  dco;
		//-------------------------------------------

		dco=(dCONNECT*)exe.append(CONNECT); 					//добавили команду
		if(port==""){
				if(nu>1 && master=="0" ){//для слейвов за мастером
					dco->cType=cTCP;
				}else{
					dco->cType=cUDP;
				}

				dco->cAddr=ip;      //адрес
				dco->cParam="502";  //порт
		}else{
				dco->cType=cCOM;
				dco->cAddr=port;
				dco->cParam=bitrate;
			}
		dco->setBAD(-1);
		//----------------------------

		for(int i=0;i<list.count();i++){

			di=(dIF*)exe.append(IF,i==0 ? "FIRST" : "");
			if(i==0) di->delay=period;
			di->setAFCmd((unsigned char)modbus.toInt(&ok,10),0x04, \
			exe.convert(lOffset.at(i)+list.at(i).toElement().attribute("id").toInt(&ok,16),2)+=exe.convert("0001"));
			di->addTest(di->getAF());


			if(i==list.count()-1){
				di->goOK_L="FIRST";
				di->goBAD_L="FIRST";
			}
			di->fOK=funcOK;
			di->fBAD=funcBAD;
		}

			//-------------------------------------------------------
		exe.Start();

		return true;
}

/*
 * Можно сгенерить сигнал из глобального сигнализатора
 */
void dataC::funcOK(quint32 ip32,const QByteArray& rez) {
	getEmit()->emitSignals(ip32,rez);
}

/*
 * Можно сгенерить сигнал из глобального сигнализатора
 */
void dataC::funcBAD(quint32 ip32,const QByteArray&) {
	getEmit()->emitSignalsBAD(ip32);
}

/*
 * Обработка данных
 */
void dataC::updateDat(quint32 ip32,const QByteArray& rez){

    if(DT==NULL)return;
    ucs dat;
    dat.ch[0]=rez.at(4);
    dat.ch[1]=rez.at(3);
    QString txt;
    txt.setNum((float)dat.sh/divider.at(CountReg),'g');
    DT->setInfo(CountReg,txt);
    CountReg++;
    if(CountReg>=DT->size()) CountReg=0;
   // qDebug()<<exe.ByteArray_to_String(rez);
}

/*
 * Обработка данных
 */
void dataC::badDat(quint32 ip32){

    if(DT!=NULL) DT->setInfo(CountReg,"???");
    CountReg++;
    if(CountReg>=DT->size()) CountReg=0;
}
/*
 *
 */

void dataC::closeEvent(QCloseEvent *event){
	emit Close();
	event->accept();
}

void dataC::Stop(){
	exe.Stop();
}

