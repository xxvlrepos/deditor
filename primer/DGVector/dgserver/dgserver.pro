TEMPLATE = app
TARGET = dgserver

CONFIG += c++11
CONFIG += console
CONFIG += serialport

win32*:{
    NAMELIB = dgsrv
    LIBDIR = libWgcc
    DEFINES += _WIN_
    OBJECTS_DIR = windows/obj
    MOC_DIR = windows/moc    
    
    win32-g++:{ DESTDIR = ../windows/WINGCC_5}
    else {     LIBDIR = libW2015
                DESTDIR ="../windows/MVC2015"

    }
}else {

 #	NAMELIB = dgsrv 
    release:{
        OBJECTS_DIR = "release/obj"
        MOC_DIR = "release/moc"
    }else{
        OBJECTS_DIR = "debug/obj"
        MOC_DIR = "debug/moc"
    }
}

QT += core \
    xml \
    xmlpatterns \
    network\
    sql \
    concurrent

    

isEmpty( NAMELIB ) {
    
        QT += serialport

         include(../server/server.pri)
	include(../execut/execut.pri) 
	include(../mdata/mdata.pri) 
	include(../libmaia/maia.pri)
	include(../com485/com485.pri)

	
	RESOURCES += "../server/server.qrc"
	
}else{
    LIBV=1
    win32:{
            win32-g++:{
                release:{LIBS += -L"../server/"$${LIBDIR}  -l$${NAMELIB}$${LIBV} }
                else{LIBS   += -L"../server/"$${LIBDIR}  -l$${NAMELIB}d$${LIBV}}
            }else{
                release:{LIBS += -L"../server/"$${LIBDIR}"" -l$${NAMELIB}$${LIBV}}
                else{LIBS   += -L"../server/"$${LIBDIR}"/Debug"   -l$${NAMELIB}d$${LIBV}}
            }
        }else{
           CONFIG (debug, debug|release) { LIBS += ../server/lib/lib$${NAMELIB}d.a }
           else {                          LIBS += ../server/lib/lib$${NAMELIB}.a }
           RESOURCES += "../server/server.qrc"
        }
        INCLUDEPATH += "../execut"
        INCLUDEPATH += "../mdata"
        INCLUDEPATH += "../libmaia"
        INCLUDEPATH += "../server"
       # INCLUDEPATH += "../qextserialport"
        INCLUDEPATH += "../qtserialport/inc"
        INCLUDEPATH += "../com485"
        INCLUDEPATH += "../server/opcua62541"
        INCLUDEPATH += "../server/opcua62541/qopen62541"
        INCLUDEPATH += "../server/opcua62541/open62541"
}  
QT -=gui


include(../qtsingleapplication/src/qtsinglecoreapplication.pri)
include(../qtservice/src/qtservice.pri)

HEADERS   += dgserver.h
SOURCES   += dgserver.cpp \
         main.cpp

