
NAMELIB=dgsrv

CONFIG += c++11


#CONFIG   += static_and_shared
unix:CONFIG   += static
win32:CONFIG   += shared
CONFIG += serialport


unix: {
	CONFIG (debug, debug|release) { TARGET = $${NAMELIB}d }
	else {                          TARGET = $${NAMELIB} }
    debug {
        OBJECTS_DIR = debug_L/obj
        MOC_DIR = debug_L/moc
    }
    release { 
        OBJECTS_DIR = release_L/obj
        MOC_DIR = release_L/moc
    }

}else{
         CONFIG (debug, debug|release){  TARGET = $$qtLibraryTarget($${NAMELIB})}
         else {                          TARGET = $$qtLibraryTarget($${NAMELIB})}
}


TEMPLATE = lib
VERSION = 1.0.0
DEFINES += DGSEVER_LIB
DEFINES += QT_BUILD_OPCUA_LIB

win32:DEFINES += _WIN_


#win32:LIBS += -lws2_32

unix: {
    DESTDIR = lib
}else{
     win32-g++: {
       DESTDIR = libWgcc
       CONFIG(release):{ DLLDESTDIR = "../windows/WINGCC_5"}

     }else{
        release:{DESTDIR = libW2015}
        DLLDESTDIR = "../windows/MVC2015"
     }
}


QT += core \
    xml \
    xmlpatterns \
    network \
    serialport

QT -= gui

include(../qtservice/src/qtservice.pri)
include(../libmaia/maia.pri)
include(../mdata/mdata.pri)
include(../execut/execut.pri)
include(server.pri)
include(../com485/com485.pri)


RESOURCES += server.qrc




