/*
 * opcuaServer.h
 *
 *  Created on: 15 марта 2019 г.
 *      Author: Usach
 */

#ifndef SERVER_OPCUA62541_OPCUASERVER_H_
#define SERVER_OPCUA62541_OPCUASERVER_H_

#include <QtCore>
#include <QThread>
#include "mMap.h"
#include <QDomNode>
#include <qopen62541.h>
#include <QtOpcUa/qopcuatype.h>


//служебные словари
//********************************************************************


//************************************************************************

//*********************
class opcuaServer;  //предварительно
//***********************

class UA :public  QObject{

	Q_OBJECT

public:
	class teg{
	public:
			teg(unsigned short a,short v=0) {addr=a; value=v;}
	        ~teg(){
	        	UA_NodeId_deleteMembers(&opcNode);
	        }
			short value;
			unsigned short addr;
			unsigned short id{0};
			unsigned short div{1};
			unsigned char  num{0};
			QChar          kType;
			UA_NodeId      opcNode;
		};

		class mdCan : public QHash<unsigned short,teg>{
		   public:
			iterator insert(unsigned short a,short v){
				return QHash::insert(a,teg(a,v));
			}
			iterator insert(unsigned short a,teg t){
				  t.addr=a;
				  return QHash::insert(a,t);
			}
		};
		class mData : public QHash<unsigned char,mdCan* >{

			public:
			virtual ~mData(){clear();}
			void clear(){
				 QList<unsigned char> k=keys();
				 for(int i=0; i<k.size();i++)   delete(value(k[i]));
				 QHash<unsigned char,mdCan* >::clear();
			}
			int remove(const unsigned char& key){
				delete(value(key));
				return QHash<unsigned char,mdCan* >::remove(key);
			}
		};

		struct dStat{

			dStat(UA_NodeId n,bool s=false){
				stat=s;
				opcNode=n;
			}

			virtual ~dStat(){UA_NodeId_deleteMembers(&opcNode);}
			bool stat;
			UA_NodeId  opcNode;
		};

		UA(mMap* m, opcuaServer *sr, QObject *parent=0);
		~UA();
signals:
	void isStoped();


public slots:
	void genData(int m);
	void stop();


private slots:
  		void update();
  		void updateDat();


private:

	   mMap* map{nullptr};
	   UA_ServerConfig *m_config{nullptr};
	   UA_Server *m_server{nullptr};
	   UA_UsernamePasswordLogin*  userPass{nullptr};
	   QString user,passwd;
	   opcuaServer* ser;  //из него только берем настройки
	   int ns=1;
	   QDomElement                   rootNode; //корневой узел модбаса
	   QHash<uchar,QDomNode> mapNode;//список узлов с описанием типа канала

	   mData                  memDat;//храним предыдущие значения
	   QHash<ushort,dStat> actData; //храним статус активности ?
	   QTimer* m_timer{nullptr};
	   QTimer* d_timer{nullptr};

	   QHash<ushort,QList<int>>  emulList;//тут сохраняем идентификаторы параметров для последующего удаления

	   UA_NodeId rootObject;
	   quint32 genNumEn(quint32,quint8,const uchar);
	   QString infToString(uchar t);
	   uchar strToInf(const QString& t);
	   void addNodesEmul(int modbus,uchar t,int i);
	   void deleteNodes(int modbus,uchar t,int i);
	   void emulNode(int modbus,bool add);
	   void addCan(mdCan* curCan,int modbus,uchar t,int i,const QList<ushort>& readDR,const QList<ushort>& readSR);
	   void genDataList(int m);
	   void testData(unsigned char);
	   void updateStat(unsigned char);


	   UA_NodeId addFolder(const UA_NodeId &folder, UA_UInt32 id, const QString &name,
				const QString &description = QString());

	   UA_NodeId addVariable(const UA_NodeId &folder, UA_UInt32 id,
			   const QVariant &value,QOpcUa::Types type,
			   const QString &name,	const QString &description = QString());

	   UA_StatusCode setVariableFloat(UA_NodeId& id, float val,UA_StatusCode st=UA_STATUSCODE_GOOD);
	   UA_StatusCode setVariableBool(UA_NodeId&  id, bool  val,UA_StatusCode st=UA_STATUSCODE_GOOD);
	   UA_StatusCode setVariableInt(UA_NodeId& id, int val,UA_StatusCode st=UA_STATUSCODE_GOOD);
	   void setStatusCodeNode(UA_NodeId& id,UA_StatusCode st=UA_STATUSCODE_GOOD);


};

//*******************************************************************
class opcThread : public QThread
{
    Q_OBJECT

public:

	opcThread(mMap* m, opcuaServer *s, QObject *parent=0):QThread(parent){
		map=m;
		srv=s;
	}
	~opcThread(){emit stop();};
	void run();

signals:
 void  stop();

public slots:


private:
  opcuaServer* srv;
   UA*    opsua{nullptr};
   mMap* map{nullptr};

   signals:
     void stopUA();
};

//*************************************************************************************

class opcuaServer : public QObject {

	Q_OBJECT

public:

	friend UA;

	opcuaServer(mMap* m, QDomDocument doc,QObject *parent=0);
	virtual ~opcuaServer();
	void start(QDomElement);

	int lport;
	int maxNumC;


public slots:
	void stop();


signals:
	void genData(int);

protected:
	mMap* map{nullptr};
	bool work{false};

	QDomElement ser;     //настройки сервер
	QDomDocument xmldoc; //настройки модбаса


	struct LP{
		LP(QString l,QString p){login=l; passwd=p;}
		QString login,passwd;
	};
    QList<LP> UserPasswd;
    opcThread*   ThredRec{nullptr};

private:

    signals:
	void stopAll();
};

#endif /* SERVER_OPCUA62541_OPCUASERVER_H_ */
