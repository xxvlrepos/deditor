HEADERS += $$PWD/qextserialport.h 
SOURCES += $$PWD/qextserialport.cpp 
unix:SOURCES += $$PWD/posix_qextserialport.cpp
win32 { 
    SOURCES += $$PWD/win_qextserialport.cpp
    DEFINES += WINVER=0x0501 # needed for mingw to pull in appropriate dbt business...probably a better way to do this
    LIBS += -lsetupapi    
    DEFINES += _TTY_WIN_
}else{
    DEFINES += _TTY_POSIX_
}