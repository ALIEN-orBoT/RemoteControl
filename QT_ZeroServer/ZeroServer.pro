#-------------------------------------------------
#
# Project created by QtCreator 2016-12-18T15:52:26
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ZeroServer
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    tcpserver.cpp \
    tcpsocket.cpp \
    zeroserver.cpp \
    zeroclient.cpp \
    screenspy.cpp \
    keyboardspy.cpp \
    filespy.cpp \
    filetransfer.cpp \
    cmdspy.cpp

HEADERS  += widget.h \
    tcpserver.h \
    tcpsocket.h \
    zeroserver.h \
    zeroclient.h \
    screenspy.h \
    keyboardspy.h \
    filespy.h \
    filetransfer.h \
    cmdspy.h

FORMS    += widget.ui

RESOURCES += \
    res.qrc

RC_FILE += winapp.rc
