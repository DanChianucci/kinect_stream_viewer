#-------------------------------------------------
#
# Project created by QtCreator 2013-07-20T23:36:02
#
#-------------------------------------------------

QT       += core gui network widgets opengl

QMAKE_CXXFLAGS += -Wall

TARGET = 3DStreamViewer
TEMPLATE = app

SOURCES += main.cpp\
        MainWindow.cpp \
    Streamer.cpp \
    CloudOpenGLPanel.cpp \
    errordialog.cpp

HEADERS  += MainWindow.h \
    Streamer.h \
    DebugUtil.h \
    CloudOpenGLPanel.h\
    DebugUtil.h \
    errordialog.h

FORMS    += MainWindow.ui \
    errordialog.ui
