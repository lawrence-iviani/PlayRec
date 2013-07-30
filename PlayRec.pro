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

SOURCES +=  source/playrecutils.cpp \
    source/playrec.cpp \
    source/rec.cpp \
    source/play.cpp \


HEADERS += source/playrec_global.h \
    source/playrecutils.h \
    source/playrec.h\
    source/rec.h \
    source/play.h \

QMAKE_CLEAN += *.o *.dll *.so *.a *.dylib


CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/build/debug
    OBJECTS_DIR = $$PWD/build/debug/obj
    MOC_DIR = $$PWD/build/debug/moc
} else {
    DESTDIR = $$PWD/build/release
    OBJECTS_DIR = $$PWD/build/release/obj
    MOC_DIR = $$PWD/build/release/moc
}

unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

