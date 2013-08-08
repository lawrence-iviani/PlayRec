#-------------------------------------------------
#
# Project created by QtCreator 2013-07-01T23:39:28
#
#-------------------------------------------------

QT       -= gui
QT       += multimedia

TARGET = PlayRec
TEMPLATE = lib

DEFINES += PLAYREC_LIBRARY

SOURCES +=  playrecutils.cpp \
    playrec.cpp \
    rec.cpp \
    play.cpp \


HEADERS += playrec_global.h \
    playrecutils.h \
    playrec.h\
    rec.h \
    play.h \


CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/../build/debug
    OBJECTS_DIR = $$PWD/../build/debug/obj
    MOC_DIR = $$PWD/build/../debug/moc
    CLEANDIROBJ =  $$PWD/../build/debug/obj
    CLEANDIR = $$PWD/../build/debug
} else {
    DESTDIR = $$PWD/../build/release
    OBJECTS_DIR = $$PWD/../build/release/obj
    MOC_DIR = $$PWD/../build/release/moc
    CLEANDIROBJ =  $$PWD/../build/release/obj
    CLEANDIR = $$PWD/../build/release
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

QMAKE_CLEAN += $$CLEANDIROBJ/*.o
QMAKE_CLEAN += $$CLEANDIR/*.dll $$CLEANDIR/*.so $$CLEANDIR/*.a $$CLEANDIR/*.dylib

