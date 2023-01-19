INCLUDEPATH += $$PWD

include($$PWD/opcua62541/opcuaServer.pri)

QT += sql
HEADERS += $$PWD/dataTablePostgreSQL.h \
	$$PWD/dataTableMySQL.h \
    $$PWD/DBTable.h \
    $$PWD/sdiagCollector.h \
    $$PWD/dgserver_global.h \
    $$PWD/logFile.h \
    $$PWD/const.h \
    $$PWD/server.h \
    $$PWD/dbOSCFile.h \
    $$PWD/dataCollector.h \
    $$PWD/dbConnector.h \
    $$PWD/Sheduler.h \
    $$PWD/ExeThread.h \
    $$PWD/ExeListThread.h \
    $$PWD/oscCollector.h \
    $$PWD/mMap.h \
    $$PWD/dataServer.h \
    $$PWD/mdbServer.h \
    $$PWD/snifferThread.h
    
SOURCES += $$PWD/DBTable.cpp \
    $$PWD/sdiagCollector.cpp \
    $$PWD/const.cpp \
    $$PWD/logFile.cpp \
    $$PWD/server.cpp \
    $$PWD/dbOSCFile.cpp \
    $$PWD/dataCollector.cpp \
    $$PWD/dbConnector.cpp \
    $$PWD/Sheduler.cpp \
    $$PWD/ExeThread.cpp \
    $$PWD/ExeListThread.cpp \
    $$PWD/oscCollector.cpp \
    $$PWD/mMap.cpp \
    $$PWD/dataServer.cpp \
    $$PWD/mdbServer.cpp \
    $$PWD/snifferThread.cpp
