#include "remote.h"
#include <QTextDocument>
#include "config.h"

Remote::Remote()
    : QObject(nullptr), reply(nullptr), waiting(false), waitingFor(0)
{
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(finished(QNetworkReply*)));
    theCodec = config->getStr("remote", "codec", "utf8");
}

static QString arg(QString str, const QStringList &ls)
{
    for(int i = 0; i < ls.size(); i++)
    {
        QString before = "~" + QString::number(i);
        str = str.replace(before, ls[i]);
    }
    return str;
}

static QString arg(QString str, const QString &s)
{
    return str.replace("~0", s);
}


void Remote::getDeck(QString id, QString _name)
{
    if(waiting)
    {
        return;
    }
    waiting = true;
    currentUrl = arg(config->getStr("remote", "getdeck", ""), id);
    waitingFor = 0;
    name = _name;
    get();
}

void Remote::error(QNetworkReply::NetworkError err)
{
    if(err == QNetworkReply::OperationCanceledError)
    {
        return;
    }
    const auto &conf = manager->activeConfiguration();
    qDebug() << conf.name() << conf.bearerTypeName();
    qDebug() << conf.isValid();
    qDebug() << conf.state();
    qDebug() << err << reply->errorString();
}

void Remote::getList(int page)
{
    if(waiting)
    {
        return;
    }
    waiting = true;
    currentUrl = arg(config->getStr("remote", "getlist", ""), QString::number(page));
    waitingFor = 1;
    get();
}

void Remote::getName(int id)
{
    if(waiting)
    {
        return;
    }
    waiting = true;
    currentUrl = arg(config->getStr("remote", "getname", ""), QString::number(id));
    waitingFor = 2;
    get();
}

void Remote::listFinished(QNetworkReply *reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(statusCode == 302)
    {
        currentUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
        waiting = true;
        get();
    }
    else
    {
        QTextCodec *codec = QTextCodec::codecForName(theCodec.toLatin1().data());
        QString page = codec->toUnicode(reply->readAll());
        QRegExp regexp(config->getStr("remote", "finishlist", ""));

        QString idI = config->getStr("remote", "deckid", "");
        QString nameI = config->getStr("remote", "deckname", "");

        regexp.setMinimal(true);
        int pos = 0, prevPos = -1;

        auto ls = QSharedPointer<QList<QPair<QString, QString> > >::create();

        while(pos >= 0 && pos > prevPos)
        {
            prevPos = pos;
            pos = regexp.indexIn(page, pos);
            pos += regexp.matchedLength();
            if(pos >= 0 && regexp.captureCount() > 0)
            {
                QString name = arg(nameI, regexp.capturedTexts());
                QTextDocument text;
                text.setHtml(name);
                name = text.toPlainText();
                ls->append(qMakePair(name, arg(idI, regexp.capturedTexts())));
            }
        }
        emit list(ls);
        emit finished();
        emit ready(true);
    }
}

void Remote::deckFinished(QNetworkReply *reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(statusCode == 302)
    {
        currentUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
        waiting = true;
        get();
    }
    else
    {
        QTextCodec *codec = QTextCodec::codecForName(theCodec.toLatin1().data());
        QString page = codec->toUnicode(reply->readAll());
        QRegExp regexp(config->getStr("remote", "finishdeck", ""));
        QString deckI = config->getStr("remote", "deck", "");
        regexp.setMinimal(true);

        regexp.indexIn(page);

        QString deck = arg(deckI, regexp.capturedTexts());

        emit deckStream(deck, name, false);
        emit finished();
        emit ready(true);
    }
}

void Remote::nameFinished(QNetworkReply *reply)
{
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if(statusCode == 302)
    {
        currentUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
        waiting = true;
        get();
    }
    else
    {
        QTextCodec *codec = QTextCodec::codecForName(theCodec.toLatin1().data());
        QString page = codec->toUnicode(reply->readAll());
        QRegExp regexp(config->getStr("remote", "finishname", ""));
        regexp.setMinimal(true);
        regexp.indexIn(page);

        QString nameI = config->getStr("remote", "name", "");

        QString name = arg(nameI, regexp.capturedTexts());
        emit cardName(name);
        emit finished();
        emit ready(true);
    }
}

void Remote::finished(QNetworkReply *reply)
{
    if(!waiting)
    {
        return;
    }

    waiting = false;
    switch(waitingFor)
    {
    case 0:
        deckFinished(reply);
        break;
    case 1:
        listFinished(reply);
        break;
    case 2:
        nameFinished(reply);
        break;
    default:
        break;
    }

    reply->deleteLater();
}

void Remote::get()
{
    emit ready(false);

    QNetworkConfigurationManager confManager;
    confManager.updateConfigurations();
    foreach(auto &conf, confManager.allConfigurations())
    {
        if(conf.state() == QNetworkConfiguration::Active)
        {
            manager->setConfiguration(conf);
            break;
        }
    }
    reply = manager->get(QNetworkRequest(currentUrl));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(error(QNetworkReply::NetworkError)));
}

void Remote::abort()
{
    if(waiting)
    {
        reply->abort();
        waiting = false;
        emit ready(true);
    }
}

Remote::~Remote()
{
    manager->deleteLater();
}

