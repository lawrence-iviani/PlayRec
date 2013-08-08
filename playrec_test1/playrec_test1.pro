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

INCLUDEPATH += $$PWD/../PlayRecLib

QMAKE_CLEAN += -r *.o *.dll *.so *.a *.dylib *.app

CONFIG(debug, debug|release) {
    DESTDIR = $$PWD/../build/debug
    OBJECTS_DIR = $$PWD/../build/debug/obj
    MOC_DIR = $$PWD/../build/debug/moc
    CLEANDIROBJ =  $$PWD/../build/debug/obj
    CLEANDIR = $$PWD/../build/debug
    macx {
        POSTLINKDIR = $$PWD/../build/debug
        LIBS += -L$$PWD/../build/debug/
        LIBS += -lPlayRec

    }
} else {
    DESTDIR = $$PWD/../build/release
    OBJECTS_DIR = $$PWD/../build/release/obj
    MOC_DIR = $$PWD/../build/release/moc
    CLEANDIROBJ =  $$PWD/../build/release/obj
    CLEANDIR = $$PWD/../build/release
    macx {
        POSTLINKDIR = $$PWD/../build/release
        LIBS += -L$$PWD/../build/release/
        LIBS += -lPlayRec
    }
}

macx {
    QMAKE_POST_LINK += $$quote(cp $$POSTLINKDIR/*.dylib $$POSTLINKDIR/playrec_test1.app/Contents/MacOS/)
}

QMAKE_CLEAN += $$CLEANDIR/*.dll $$CLEANDIR/*.so $$CLEANDIR/*.a $$CLEANDIR/*.dylib
QMAKE_CLEAN += $$CLEANDIROBJ/*.o


