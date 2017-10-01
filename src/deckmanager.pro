#-------------------------------------------------
#
# Project created by QtCreator 2015-05-30T22:15:53
#
#-------------------------------------------------

QT       += core gui
CONFIG   += c++14

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = deckmanager
TEMPLATE = app

DEFINES   += QUAZIP_STATIC

win32:{
INCLUDEPATH += D:\dev\quazip-0.7.3\quazip-static\include
INCLUDEPATH += D:\dev\curl\include
INCLUDEPATH += D:\dev\ChezScheme-master\ti3nt\boot\ti3nt
LIBS += D:\dev\ChezScheme-master\ti3nt\bin\ti3nt\csv941.dll
#LIBS += -LD:\dev\curl  -LD:\dev\openssl-0.9.8h-1-lib\lib -lcurl -lssh2 -lrtmp -lssl -lcrypto -lidn -lws2_32 -lwldap32 -lwinmm
LIBS += -LD:\dev\curl  -LD:\dev\openssl-1.0.2l\dist\lib -lcurl -lssl -lcrypto -lws2_32 -lwldap32 -lwinmm -lcrypt32
LIBS += -LD:\dev\build-quazip-static-Release\quazip\release -lquazip
LIBS += -LD:\dev\zlib-1.2.11 -lz
QMAKE_LFLAGS += -static
DEFINES += CURL_STATICLIB
}

SOURCES += main.cpp\
        mainwindow.cpp \
    carditem.cpp \
    deckwidget.cpp \
    deckview.cpp \
    card.cpp \
    cardslist.cpp \
    decklist.cpp \
    cardpicture.cpp \
    carddetails.cpp \
    locallist.cpp \
    cardfilter.cpp \
    yrp.cpp \
    lzma/LzmaDec.c \
    replaylist.cpp \
    scriptview.cpp \
    packlist.cpp \
    packedit.cpp \
    packview.cpp \
    expansions.cpp \
    limitcards.cpp \
    pref.cpp \
    mtabwidget.cpp \
    deckmodel.cpp \
    sqlite3/sqlite3.c \
    engine.cpp \
    networking.cpp \
    configmanager.cpp

HEADERS  += mainwindow.h \
    carditem.h \
    deckwidget.h \
    deckview.h \
    card.h \
    cardslist.h \
    decklist.h \
    cardpicture.h \
    carddetails.h \
    locallist.h \
    cardfilter.h \
    yrp.h \
    lzma/LzmaDec.h \
    lzma/Types.h \
    replaylist.h \
    scriptview.h \
    packlist.h \
    packedit.h \
    packview.h \
    expansions.h \
    limitcards.h \
    pref.h \
    iconbutton.h \
    mtabwidget.h \
    decklabel.h \
    deckmodel.h \
    signaltower.h \
    sqlite3/sqlite3.h \
    arrange.h \
    engine.h \
    networking.h \
    common.h \
    configmanager.h \
    types.h

RESOURCES += \
    help.qrc
