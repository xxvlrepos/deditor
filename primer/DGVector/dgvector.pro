TEMPLATE = app
TARGET = dgvector


CONFIG += c++11


win32:DEFINES += _WIN_
win32 { 
    
    NAMELIB = dgsrv
    LIBDIR = libWgcc
    OBJECTS_DIR = windows/obj
    MOC_DIR = windows/moc
    win32-g++ { 
        # LIBS += -lws2_32
       # DEFINES += _WINGCC_
       DESTDIR = ./windows/WINGCC_5
    }else {    
        LIBDIR = libW2015
        DESTDIR = windows/MVC2015
    }
} else {
   #NAMELIB = dgsrv
    debug { 
        OBJECTS_DIR = debug/obj
        MOC_DIR = debug/moc
    }
    release { 
        OBJECTS_DIR = release/obj
        MOC_DIR = release/moc
    }
}

QT += core \
    gui \
    xml \
    xmlpatterns \
    network \
    sql \
    widgets\
    printsupport \
    serialport

    
isEmpty( NAMELIB ) { 
    include(server/server.pri)   
    include(qtservice/src/qtservice.pri)
    include(execut/execut.pri)
    include(mdata/mdata.pri)
    include(libmaia/maia.pri)
    include(com485/com485.pri)       
    RESOURCES += ./server/server.qrc
    
}else{
    LIBV=1
    win32:{
            win32-g++: {
                release:{LIBS += -L"./server/"$${LIBDIR}  -l$${NAMELIB}$${LIBV}}
                else{LIBS   += -L"./server/"$${LIBDIR}  -l$${NAMELIB}d$${LIBV}}
            }else{
                release:{LIBS += -L"./server/"$${LIBDIR}"" -l$${NAMELIB}$${LIBV}}
                else{LIBS   += -L"./server/"$${LIBDIR}"/Debug"   -l$${NAMELIB}d$${LIBV}}
            }
        }else{
           CONFIG (debug, debug|release) { LIBS += ./server/lib/lib$${NAMELIB}d.a }
           else {                          LIBS += ./server/lib/lib$${NAMELIB}.a }
           RESOURCES += "./server/server.qrc"
        }
	
	INCLUDEPATH += "./execut"
	INCLUDEPATH += "./mdata"	
	INCLUDEPATH += "./libmaia"	
	INCLUDEPATH += "./server"
        INCLUDEPATH += "./qtserialport/inc"
        INCLUDEPATH += "./com485"
    
       INCLUDEPATH += "./server/opcua62541"
       INCLUDEPATH += "./server/opcua62541/qopen62541"
       INCLUDEPATH += "./server/opcua62541/open62541"

}  

    

    include(deditor/deditor.pri)
    include(former/former.pri)
    include(helpbrowser/helpBrowser.pri)
    include(hexspinbox/hexspinbox.pri)
    include(parser/parser.pri)
    include(qtsingleapplication/src/qtsingleapplication.pri)
    include(search/search.pri)
    include(xmlConverter/xmlConverter.pri)
    include(tunerWin/tunerwin.pri)


    HEADERS +=  dgvector.h 

    SOURCES +=  main.cpp \
                dgvector.cpp
        
    RESOURCES += dgvector.qrc
    TRANSLATIONS += resource/dgvector_ru.ts



