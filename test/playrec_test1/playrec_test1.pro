#-------------------------------------------------
#
# Project created by QtCreator 2013-07-27T19:54:23
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = playrec_test1
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

INCLUDEPATH += ../../Source

macx {
    # Link to playrec framework
    LIBS += -F../Debug/
    LIBS += -framework PlayRec
} else {
    LIBS += -L../Debug/
    LIBS += -lPlayRec
}
