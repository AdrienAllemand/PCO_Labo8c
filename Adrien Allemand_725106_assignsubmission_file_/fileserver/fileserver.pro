QT       += core websockets
QT       -= gui

TARGET = fileserver
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

SOURCES += \
    main.cpp \
    fileserver.cpp \
    filereader.cpp \
    requesthandler.cpp \
    responsedispatcherthread.cpp \
    requestdispatcherthread.cpp \
    concretebuffer.cpp \
    threadpool.cpp \
    hoaremonitor.cpp \
    readerwritercache.cpp \
    readerwriterlock.cpp

HEADERS += \
    fileserver.h \
    filereader.h \
    response.h \
    abstractbuffer.h \
    request.h \
    requesthandler.h \
    responsedispatcherthread.h \
    requestdispatcherthread.h \
    concretebuffer.h \
    threadpool.h \
    runnablerequest.h \
    runnable.h \
    slavethread.h \
    hoaremonitor.h \
    option.h \
    readerwritercache.h \
    readerwriterlock.h

EXAMPLE_FILES += fileclient.html

