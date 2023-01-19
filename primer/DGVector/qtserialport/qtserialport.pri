
include($$PWD/src/serialport/serialport-lib.pri)

equals(QT_MAJOR_VERSION, 4) {
	include($$PWD/qt4support.pri) 
}

INCLUDEPATH += $$PWD/inc

DEFINES += QT_STATIC

win32-g++:{
  INCLUDEPATH +="C:\Qt\qt-4.8.5-x86-mingw\mingw32\i686-w64-mingw32\include"
}

win32 { 
    DEFINES += _TTY_WIN_
}else{
    DEFINES += _TTY_POSIX_
}
