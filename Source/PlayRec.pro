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


unix:!symbian {
    maemo5 {
        target.path = /opt/usr/lib
    } else {
        target.path = /usr/lib
    }
    INSTALLS += target
}

macx {
    CONFIG += lib_bundle
}
