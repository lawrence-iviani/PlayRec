#-------------------------------------------------
#
# Project created by QtCreator 2013-07-27T19:54:23
#
#-------------------------------------------------

QT       += core gui multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = playrec_test1
TEMPLATE = app


SOURCES += playrec_test1/main.cpp\
        playrec_test1/mainwindow.cpp

HEADERS  += playrec_test1/mainwindow.h

FORMS    += playrec_test1/mainwindow.ui

INCLUDEPATH += $$PWD/source


CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/build/debug
    OBJECTS_DIR = $$PWD/build/debug/obj
    MOC_DIR = $$PWD/build/debug/moc
    macx {
        LIBS += -L$$PWD/build/debug/
        LIBS += -lPlayRec
        QMAKE_POST_LINK += $$quote(cp $$PWD/build/debug/*.dylib $$PWD/build/debug/playrec_test1.app/Contents/MacOS/)
    }
} else {
    DESTDIR = $$PWD/build/release
    OBJECTS_DIR = $$PWD/build/release/obj
    MOC_DIR = $$PWD/build/release/moc
    macx {
        LIBS += -L$$PWD/build/release/
        LIBS += -lPlayRec
        QMAKE_POST_LINK += $$quote(cp $$PWD/build/release/*.dylib $$PWD/build/release/playrec_test1.app/Contents/MacOS/)
    }
}


