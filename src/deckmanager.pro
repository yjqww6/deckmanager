#-------------------------------------------------
#
# Project created by QtCreator 2015-05-30T22:15:53
#
#-------------------------------------------------

QT       += core gui
CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = deckmanager
TEMPLATE = app

DEFINES   += QUAZIP_STATIC

win32:{
INCLUDEPATH += D:\Qt\quazip-static\include
INCLUDEPATH += D:\C\curl\include
INCLUDEPATH += D:\C\ti3nt\boot\ti3nt
LIBS += D:\C\ti3nt\bin\csv94.dll
LIBS += -LD:\C\curl\lib -lcurl -lssh2 -lrtmp -lssl -lcrypto -lidn -lws2_32 -lwldap32 -lwinmm
LIBS += -LD:\Qt\build-quazip-Desktop_Qt_5_6_0_MinGW_32bit_1e8aa2-Release\quazip\release -lquazip
LIBS += -LD:\Qt\zlib-1.2.8\build -lzlibstatic
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
    config.cpp \
    mtabwidget.cpp \
    deckmodel.cpp \
    sqlite3/sqlite3.c \
    engine.cpp \
    networking.cpp

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
    config.h \
    iconbutton.h \
    draghelper.h \
    mtabwidget.h \
    range.h \
    typing.h \
    wrapper.h \
    decklabel.h \
    deckmodel.h \
    signaltower.h \
    sqlite3/sqlite3.h \
    arrange.h \
    engine.h \
    networking.h

RESOURCES += \
    help.qrc
