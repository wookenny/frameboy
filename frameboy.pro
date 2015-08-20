#-------------------------------------------------
#
# Project created by QtCreator 2015-07-16T17:34:11
#
#-------------------------------------------------

QT       += core gui svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = frameboy
TEMPLATE = app
CONFIG += c++11
QMAKE_CXXFLAGS += -std=c++11

DEFINES += GIT_CURRENT_SHA1="\\\"$(shell git -C \""$$_PRO_FILE_PWD_"\" rev-parse --short  HEAD)\\\""


INCLUDEPATH += ./src

SOURCES += src/main.cpp\
        src/mainwindow.cpp \
    src/videowriter.cpp \
    src/common.cpp \
    src/graphicsviewscaling.cpp \
    src/graphicspixmapitemwatermark.cpp

HEADERS  += src/mainwindow.h \
    src/videowriter.h \
    src/common.h \
    src/graphicsviewscaling.h \
    src/graphicspixmapitemwatermark.h

FORMS    += src/mainwindow.ui 



#unix {
   CONFIG += link_pkgconfig
   PKGCONFIG += opencv
   LIBS += -L/usr/local/lib
#}

RESOURCES += \
    icons.qrc

OTHER_FILES +=

