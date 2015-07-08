#ifndef REMOTE_H
#define REMOTE_H

#include <QObject>
#include <QTextCodec>
#include <QTextStream>
#include <QByteArray>
#include <QRegExp>
#include <QList>
#include <QPair>
#include <QSharedPointer>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QNetworkConfiguration>
#include <QtNetwork/QNetworkConfigurationManager>
#include <QDebug>
#include "typing.h"

class Remote : public QObject
{
    Q_OBJECT
public:
    explicit Remote();

    ~Remote();

    bool is_waiting()
    {
        return waiting;
    }


signals:

    void deckStream(QString, QString, bool);
    void list(Type::DeckL);
    void cardName(QString);
    void finished();
    void ready(bool);
public slots:

    void getDeck(QString, QString);
    void getList(int page = 1);
    void getName(quint32);
    void abort();
    void finished(QNetworkReply *);
    void error(QNetworkReply::NetworkError);
private:
    void listFinished(QNetworkReply *);
    void deckFinished(QNetworkReply *);
    void nameFinished(QNetworkReply *);
    void get();
    QNetworkReply *reply;
    QNetworkAccessManager *manager;
    QUrl currentUrl;
    bool waiting;
    int waitingFor;
    QString name;
    QString theCodec;
};

#endif // REMOTE_H
