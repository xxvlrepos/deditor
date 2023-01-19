
#ifndef DATATABLEMYSQL_H_
#define DATATABLEMYSQL_H_
#include "DBTable.h"
#include "dgserver_global.h"

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

class UpdTrigerM: public DBTriger{
  public:
	UpdTrigerM() :  DBTriger("opdat_after_update","opdat","hidat"){
		NInsert="opdat_after_insert";
	};
	~UpdTrigerM(){};
	QString greate(){
		QString str ="CREATE TRIGGER `"+Name+"` AFTER UPDATE ON `"+SrcOb+"` FOR EACH ROW BEGIN \n"
				"INSERT IGNORE INTO `"+RezOb+"` VALUES (NEW.modbus, NEW.addr, NEW.value, NEW.ctime);\n"
				"END";
		return str;
	}
	QString greateF(){
		QString str ="CREATE TRIGGER `"+NInsert+"` AFTER INSERT ON `"+SrcOb+"` FOR EACH ROW BEGIN \n"
				"INSERT IGNORE INTO `"+RezOb+"` VALUES (NEW.modbus, NEW.addr, NEW.value, NEW.ctime);\n"
				"END";
		return str;
	}
	QString NInsert;
};

class ClearProcM: public DBProc{

  public:
	ClearProcM() :  DBProc("clear_proc"){};
	~ClearProcM(){};
	QString greate(){
		QString str;
		str =   "CREATE  PROCEDURE `" +Name+"`(IN `deep_min` INT) "
				    "LANGUAGE SQL "
					"NOT DETERMINISTIC "
					"CONTAINS SQL "
					"SQL SECURITY DEFINER "
					"COMMENT '' "
				"BEGIN \n"
				"DECLARE ct  TIMESTAMP DEFAULT (NOW() - INTERVAL deep_min MINUTE);\n"
				"DECLARE tm  TIMESTAMP DEFAULT (SELECT MIN(ctime) FROM hidat);\n"
				"IF (tm<=ct) THEN  \n"
				" SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;\n"
				" START TRANSACTION;\n"
				" DELETE FROM hidat WHERE hidat.ctime <= ct;\n"
				" INSERT INTO hidat SELECT opdat.modbus, opdat.addr, opdat.value, NOW() \n"
				"      FROM opdat LEFT JOIN hidat  ON opdat.modbus = hidat.modbus and opdat.addr = hidat.addr \n"
				"	   WHERE hidat.modbus is null And hidat.addr is null; \n"
				" COMMIT;\n"
				"END IF;\n"
				"END";
		return str;
	}

	QString drop(){return "DROP PROCEDURE "+Name+";";};
	QString call(QString p){return "CALL "+Name+"("+p+");";};
};

//таблица активности сервера
class TableSER : public DBTableM{
public:
	TableSER() : DBTableM("server"){
		addColum("key",  "TINYINT UNSIGNED NOT NULL",1);
		addColum("ctime","TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP");
		Prop="COLLATE='utf8_general_ci' ENGINE=MEMORY";
	}
	QString update(){
		//return "UPDATE `"+Name+"` SET `ctime` = DEFAULT WHERE `key` = 1;\n";
		return "UPDATE `"+Name+"` SET `ctime` = now() WHERE `key` = 1;\n";
	}
};

//таблица активности приборов
class TableACT : public DBTableM{
public:
	TableACT() : DBTableM("status"){
		addColum("modbus", "TINYINT UNSIGNED NOT NULL",1);
		addColum("state",  "TINYINT DEFAULT '0'");
		addColum("ctime",  "TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP");
		addColum("typeCon","CHAR(7)");
		addColum("connect","CHAR(16)");
		addColum("alias",  "CHAR(17)");
		Prop="COLLATE='utf8_general_ci' ENGINE=MEMORY";
	}
};

//таблица оперативных данных
class TableTD : public DBTableM{
public:
	TableTD() : DBTableM("opdat"){
		addColum("modbus","TINYINT UNSIGNED NOT NULL", 1);
		addColum("addr",  "SMALLINT UNSIGNED NOT NULL",2);
		addColum("value", "SMALLINT DEFAULT '0'");
		addColum("ctime", "TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP");
		Prop="COLLATE='utf8_general_ci' ENGINE=MEMORY";
	}
};

//таблица истории
class TableHD : public DBTableM{
public:
	TableHD() : DBTableM("hidat"){
		addColum("modbus","TINYINT UNSIGNED NOT NULL", 2);
		addColum("addr",  "SMALLINT UNSIGNED NOT NULL",3);
		addColum("value", "SMALLINT DEFAULT '0'");
		addColum("ctime", "TIMESTAMP NOT NULL",         1);
		Prop="COLLATE='utf8_general_ci' ENGINE=MEMORY";
	}
};

//таблица статических данных
class TableSTD : public DBTableM{
public:
	TableSTD() : DBTableM("stdat"){
		addColum("modbus", "TINYINT UNSIGNED NOT NULL", 1);
		addColum("addr",   "SMALLINT UNSIGNED NOT NULL",2);
		addColum("value",  "SMALLINT DEFAULT '0'");
		Prop="COLLATE='utf8_general_ci' ENGINE=InnoDB";
	}
};

//словарь тегов
class TableDITD : public DBTableM{
public:
	TableDITD() : DBTableM("dist"){
		addColum("modbus","TINYINT UNSIGNED NOT NULL", 1);
		addColum("addr",  "SMALLINT UNSIGNED NOT NULL",2);
		addColum("knum",  "TINYINT UNSIGNED NOT NULL");
		addColum("ktype", "CHAR(1) NOT NULL");
		addColum("id",    "SMALLINT UNSIGNED NOT NULL");
		Prop="COLLATE='utf8_general_ci' ENGINE=InnoDB";
	}
};


//описание тегов
class TableOPT : public DBTableM{
public:
	TableOPT() : DBTableM("tegsname"){
		addColum("ktype",    "CHAR(1) NOT NULL",          1);
		addColum("id",       "SMALLINT UNSIGNED NOT NULL",2);
		addColum("divider",  "TINYINT UNSIGNED NOT NULL DEFAULT '1'");
		addColum("name",     "CHAR(10)");
		addColum("dimension","CHAR(10)");
		addColum("title",    "CHAR(30)");
		Prop="COLLATE='utf8_general_ci' ENGINE=InnoDB";
	}
};

//-------------------------------------------------------------------------------
//Осцилограммы

class TableTOSC_M : public DBTableM{
public:
	TableTOSC_M() : DBTableM("tosc"){
		addColum("utime",   "INT UNSIGNED NOT NULL",3);
		addColum("modbus",  "TINYINT UNSIGNED NOT NULL",1);
		addColum("channel", "TINYINT UNSIGNED NOT NULL",2);
		addColum("dataosc", "MEDIUMBLOB");
		Prop="COLLATE='utf8_general_ci' ENGINE=InnoDB";
	}
};

//-----------------------------------------------------------------------------------
//Диагностика
/*
 * TableSData="spdat";
   TableDist="spdist";
   TableName="spname";
 */


class TableSData_M : public DBTableM{
public:
	TableSData_M() : DBTableM("spdat"){
		addColum("modbus","TINYINT UNSIGNED NOT NULL",1);
		addColum("addr",  "SMALLINT UNSIGNED NOT NULL",2);
		addColum("value", "SMALLINT DEFAULT '0'");
		addColum("ctime", "TIMESTAMP NOT NULL");
#ifdef _UNSHED_DIAG_
		Prop="COLLATE='utf8_general_ci' ENGINE=MEMORY";
#else
		Prop="COLLATE='utf8_general_ci' ENGINE=InnoDB";
#endif
	}
};

class TableDist_M : public DBTableM{
public:
	TableDist_M() : DBTableM("spdist"){
		addColum("modbus","TINYINT UNSIGNED NOT NULL",1);
		addColum("addr",  "SMALLINT UNSIGNED NOT NULL",2);
		addColum("knum",  "TINYINT UNSIGNED NOT NULL");
		addColum("ktype", "CHAR(1) NOT NULL");
		addColum("id",    "SMALLINT UNSIGNED NOT NULL");
		Prop="COLLATE='utf8_general_ci' ENGINE=InnoDB";
	}
};

class TableName_M : public DBTableM{
public:
	TableName_M() : DBTableM("spname"){
		addColum("id",     "SMALLINT UNSIGNED NOT NULL",1);
		addColum("fr_lo",  "FLOAT UNSIGNED NOT NULL");
		addColum("fr_hi",  "FLOAT UNSIGNED NOT NULL");
		addColum("fr_re",  "FLOAT UNSIGNED NOT NULL");
		addColum("divider","TINYINT UNSIGNED NOT NULL DEFAULT '1'");
		addColum("name",   "CHAR(4)");
		addColum("dimension","CHAR(10)");
		addColum("label",  "CHAR(16)");
		Prop="COLLATE='utf8_general_ci' ENGINE=InnoDB";
	}
};



#endif //DATATABLEMYSQL_H_

