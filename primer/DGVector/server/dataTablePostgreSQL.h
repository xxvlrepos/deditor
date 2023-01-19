
#ifndef DATATABLEPOSTGRESQL_H_
#define DATATABLEPOSTGRESQL_H_
#include "DBTable.h"

/*
    nameACT="status";
	nameTD="opdat";
	nameHD="hidat";
	nameSTD="stdat";
	nameDITD="dist";
	nameOPT="tegsname";

	QString nameACT;//таблица активности приборов
	QString nameTD; //таблица оперативных данных
	QString nameHD; //таблица истории
	QString nameSTD;//таблица статических данных
	QString nameDITD;//словарь тегов
	QString nameOPT;//описание тегов
 */

class UpdTrigerP: public DBTriger{
  public:
	UpdTrigerP() :  DBTriger("opdat_after_update","opdat","hidat"){
		NFunc="func_hidat_update";
	};
	~UpdTrigerP(){};
	QString NFunc;
	QString greate(){
		QString str;
		str="CREATE TRIGGER "+Name+" AFTER INSERT OR UPDATE OF ctime ON "
			+SrcOb+" FOR EACH ROW EXECUTE PROCEDURE "+NFunc +"();";
		return str;
	}
	QString greateF(){
		QString str;

		str ="CREATE OR REPLACE FUNCTION "+NFunc+"() RETURNS trigger AS \n"
		     "$BODY$\n"
			 "BEGIN\n"
		     "INSERT INTO "+RezOb+" VALUES (NEW.modbus, NEW.addr, NEW.value, NEW.ctime) \n"
		    		 "ON CONFLICT DO NOTHING;\n"
			 "RETURN NEW;\n"
		     "END;\n"
			 "$BODY$\n"
		     "LANGUAGE plpgsql VOLATILE;";
		return str;
	}



};

class ClearProcP: public DBProc{
  public:
	ClearProcP() :  DBProc("clear_proc"){};
	~ClearProcP(){};
	QString greate(){
		QString str;
		str = "CREATE  FUNCTION clear_proc(deep_min integer)  RETURNS integer AS \n"
				"$BODY$\n"
				"DECLARE \n"
				"ct  timestamp without time zone;\n"
				"tm  timestamp without time zone;\n"
				"BEGIN\n"
				"ct:=(now()- concat(deep_min,' minutes')::interval);\n"
				"tm:=(SELECT min(ctime) FROM hidat);\n"
				"IF (tm<=ct) THEN\n"
				" DELETE FROM hidat WHERE hidat.ctime <= ct;\n"
				" INSERT INTO hidat SELECT opdat.modbus, opdat.addr, opdat.value, now() \n"
				" FROM opdat LEFT JOIN hidat  ON opdat.modbus = hidat.modbus and opdat.addr = hidat.addr \n"
				" WHERE hidat.modbus is null And hidat.addr is null;\n"
				"END IF;\n"
				" RETURN  deep_min;\n"
				"END;\n"
				"$BODY$\n"
				"  LANGUAGE plpgsql VOLATILE;";
		return str;
	}
	QString drop(){return "DROP FUNCTION "+Name+"(integer);";};
	QString call(QString p){return "SELECT "+Name+"("+p+");";};
};


//таблица активности сервера
class TableSER_P : public DBTableP{
public:
	TableSER_P() : DBTableP("server"){
		Create="CREATE UNLOGGED TABLE"; //в памяти
		addColum("key", "smallint NOT NULL",1);
		addColum("ctime",  "timestamp without time zone DEFAULT now()");
	}
	QString update(){
		return "UPDATE "+Name+" SET ctime = DEFAULT WHERE key = 1;\n";
	}

};

//таблица активности приборов
class TableACT_P : public DBTableP{
public:
	TableACT_P() : DBTableP("status"){
		Create="CREATE UNLOGGED TABLE"; //в памяти
		addColum("modbus", "smallint NOT NULL",1);
		addColum("state",  "smallint DEFAULT 0");
		addColum("ctime",  "timestamp without time zone DEFAULT now()");
		addColum("typeCon","character(7)");
		addColum("connect","character(16)");
		addColum("alias",  "character(17)");
	}

};

//таблица оперативных данных
class TableTD_P : public DBTableP{
public:
	TableTD_P() : DBTableP("opdat"){
		Create="CREATE UNLOGGED TABLE"; //в памяти
		addColum("modbus","smallint NOT NULL", 1);
		addColum("addr",  "smallint NOT NULL",2);
		addColum("value", "smallint DEFAULT 0");
		addColum("ctime", "timestamp without time zone DEFAULT now()");
	}

};

//таблица истории
class TableHD_P : public DBTableP{
public:
	TableHD_P() : DBTableP("hidat"){
		Create="CREATE UNLOGGED TABLE"; //в памяти
		addColum("modbus","smallint NOT NULL", 2);
		addColum("addr",  "smallint NOT NULL",3);
		addColum("value", "smallint DEFAULT 0");
		addColum("ctime", "timestamp without time zone DEFAULT now()",         1);
	}
};

//таблица статических данных
class TableSTD_P : public DBTableP{
public:
	TableSTD_P() : DBTableP("stdat"){
		addColum("modbus", "smallint NOT NULL", 1);
		addColum("addr",   "smallint NOT NULL",2);
		addColum("value",  "smallint DEFAULT 0");
	}
};

//словарь тегов
class TableDITD_P : public DBTableP{
public:
	TableDITD_P() : DBTableP("dist"){
		addColum("modbus","smallint NOT NULL", 1);
		addColum("addr",  "smallint NOT NULL",2);
		addColum("knum",  "smallint NOT NULL");
		addColum("ktype", "character(1) NOT NULL");
		addColum("id",    "smallint NOT NULL");
	}
};


//описание тегов
class TableOPT_P : public DBTableP{
public:
	TableOPT_P() : DBTableP("tegsname"){
		addColum("ktype",    "character(1) NOT NULL",1);
		addColum("id",       "smallint NOT NULL",2);
		addColum("divider",  "smallint NOT NULL DEFAULT 1");
		addColum("name",     "character(10)");
		addColum("dimension","character(10)");
		addColum("title",    "character(30)");
	}
};

//-------------------------------------------------------------------------------
//Осцилограммы

class TableTOSC_P : public DBTableP{
public:
	TableTOSC_P() : DBTableP("tosc"){
		addColum("utime",   "integer NOT NULL",3);
		addColum("modbus",  "smallint NOT NULL",1);
		addColum("channel", "smallint NOT NULL",2);
		addColum("dataosc", "bytea");
	}
};

//-----------------------------------------------------------------------------------
//Диагностика
/*
 * TableSData="spdat";
   TableDist="spdist";
   TableName="spname";
 */


class TableSData_P : public DBTableP{
public:
	TableSData_P() : DBTableP("spdat"){
		addColum("modbus","smallint NOT NULL",1);
		addColum("addr",  "smallint NOT NULL",2);
		addColum("value", "smallint DEFAULT 0");   //типа unsigned нет!
		addColum("ctime", "timestamp without time zone DEFAULT now()");
	}
	QString replace(){//с модификацией
		 QString str="INSERT INTO "+Name+" VALUES ";
		 str.append(getVList()+" ON CONFLICT (modbus,addr) DO UPDATE SET ");
		 str.append(getColumEQ(2)+","+getColumEQ(3)+";");
		 return str;
	}
};

class TableDist_P : public DBTableP{
public:
	TableDist_P() : DBTableP("spdist"){
		addColum("modbus","smallint NOT NULL",1);
		addColum("addr",  "smallint NOT NULL",2);
		addColum("knum",  "smallint NOT NULL");
		addColum("ktype", "character(1) NOT NULL");
		addColum("id",    "smallint NOT NULL");
	}
	QString replace(){//без модификации
		 QString str="INSERT INTO "+Name+" VALUES ";
		 str.append(getVList()+" ON CONFLICT DO NOTHING;");
		 return str;
	}

};

class TableName_P : public DBTableP{
public:
	TableName_P() : DBTableP("spname"){
		addColum("id",      "smallint NOT NULL",1);
		addColum("fr_lo",   "real NOT NULL");
		addColum("fr_hi",   "real NOT NULL");
		addColum("fr_re",   "real NOT NULL");
		addColum("divider", "smallint DEFAULT 1");
		addColum("name",    "character(4)");
		addColum("dimension","character(10)");
		addColum("label",    "character(16)");
	}
};



#endif //DATATABLEPOSTGRESQL_H_

