QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = hashcatui
CONFIG += c++11

CONFIG(debug, debug|release):CONFIGURATION=debug

CONFIG(release, debug|release){
    CONFIGURATION=release
    #from static build
    QMAKE_LFLAGS_RELEASE += -static -static-libgcc
}

build_pass:CONFIG(debug, debug|release) {
    TARGET = $$join(TARGET,,,_debug)
}


DEFINES += QT_DEPRECATED_WARNINGS

OBJECTS_DIR         = ../build/obj/$${CONFIGURATION}
MOC_DIR             = ../build/$${CONFIGURATION}
RCC_DIR             = ../build/rcc
UI_DIR              = ../build/ui
DESTDIR             = ../bin

SOURCES += \
    global.cpp \
    helpwindow.cpp \
    main.cpp \
    mainwindow.cpp
HEADERS += \
    global.h \
    helpwindow.h \
    mainwindow.h
FORMS += mainwindow.ui \
    helpwindow.ui

RESOURCES += resources.qrc
TRANSLATIONS = lang/ru_RU.ts

exists(./gitversion.pri):include(./gitversion.pri)
exists(./myLibs.pri):include(./myLibs.pri)
exists(index.rc){
    win32|win64{
        RC_FILE=  index.rc
        OTHER_FILES+= index.rc
        DISTFILES += index.rc
    }
}
