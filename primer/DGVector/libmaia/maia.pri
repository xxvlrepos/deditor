INCLUDEPATH += $$PWD/
HEADERS += $$PWD/maiaObject.h \
    $$PWD/maiaFault.h \
    $$PWD/maiaXmlRpcClient.h \
    $$PWD/maiaXmlRpcServer.h \
    $$PWD/maiaCore.h
SOURCES += $$PWD/maiaObject.cpp \
    $$PWD/maiaFault.cpp \
    $$PWD/maiaXmlRpcClient.cpp \
    $$PWD/maiaXmlRpcServer.cpp \
    $$PWD/maiaCore.cpp
QT += xml \
    network
equals(QT_MAJOR_VERSION, 5): { 
    HEADERS += $$PWD/http_headers.h
    SOURCES += $$PWD/http_headers.cpp
}
