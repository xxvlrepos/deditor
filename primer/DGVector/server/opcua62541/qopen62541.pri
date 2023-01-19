INCLUDEPATH += $$PWD/qopen62541
INCLUDEPATH += $$PWD/qopen62541/QtOpcUa

include($$PWD/open62541.pri)

HEADERS += $$PWD/qopen62541/qopen62541.h \
    $$PWD/qopen62541/qopen62541utils.h \
    $$PWD/qopen62541/QtOpcUa/qopcuaglobal.h\
    $$PWD/qopen62541/QtOpcUa/qopcuatype.h \
    $$PWD/qopen62541/QtOpcUa/qopcuanodeids.h \
    $$PWD/qopen62541/QtOpcUa/qopcuabinarydataencoding.h \
    $$PWD/qopen62541/qopen62541valueconverter.h
    
SOURCES +=  $$PWD/qopen62541/qopen62541utils.cpp \
    $$PWD/qopen62541/QtOpcUa/qopcuatype.cpp \
    $$PWD/qopen62541/QtOpcUa/qopcuabinarydataencoding.cpp \
    $$PWD/qopen62541/qopen62541valueconverter.cpp
