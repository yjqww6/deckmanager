#include "remote.h"
#include <QTextDocument>
#include "config.h"
#include "range.h"

Remote::Remote()
    : QObject(nullptr), reply(nullptr), waiting(false), waitingFor(0)
{
    manager = new QNetworkAccessManager(this);
    connect(manager, static_cast<void (QNetworkAccessManager::*)(QNetworkReply*)>(&QNetworkAccessManager::finished),
            this, static_cast<void (Remote::*)(QNetworkReply*)>(&Remote::finished));
    theCodec = config->getCurrentRemote().codec;
}

static QString arg(QString str, const QStringList &ls)
{
    for(int i : range(ls.size()))
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
    currentUrl = arg(config->getCurrentRemote().getdeck, id);
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

static QString request(int decktype, int flt)
{
    QString param;
    if(flt == -1)
    {
        return config->tempConfig;
    }
    else if(decktype == 0)
    {
        param = config->remoteConfig.getlistparam;
    }
    else
    {
        param = config->remoteConfig.getlistparamwithdecktype;
    }
    QStringList querys;
    if(decktype != 0)
    {
        querys << "DeckType=" + QString::number(decktype);
    }
    if(flt != 0)
    {
        querys << "Flt=" + QString::number(flt);
    }
    QString query = querys.join('&');

    return param + query;
}

void Remote::getList(int page)
{
    if(waiting)
    {
        return;
    }
    waiting = true;
    QString param = request(config->deckType, config->Flt);
    QString getlist = config->remoteConfig.getlist;
    QStringList args;
    args << QString::number(page);
    args << param;
    currentUrl = arg(getlist, args);
    waitingFor = 1;
    get();
}

void Remote::getName(quint32 id)
{
    if(waiting)
    {
        return;
    }
    waiting = true;
    currentUrl = arg(config->getCurrentRemote().getname, QString::number(id));
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
        if(!codec)
        {
            codec = QTextCodec::codecForName("utf8");
        }
        QString page = codec->toUnicode(reply->readAll());
        QRegExp regexp(config->getCurrentRemote().finishlist);

        QString idI = config->getCurrentRemote().deckid;
        QString nameI = config->getCurrentRemote().deckname;
        QString typeI = config->getCurrentRemote().decktype;
        QString tooltipI = config->getCurrentRemote().decktooltip;

        regexp.setMinimal(true);
        int pos = 0, prevPos = -1;

        auto ls = Type::DeckL::create();

        while(pos >= 0 && pos > prevPos)
        {
            prevPos = pos;
            pos = regexp.indexIn(page, pos);
            pos += regexp.matchedLength();
            if(pos >= 0 && regexp.captureCount() > 0)
            {
                QString name = arg(nameI, regexp.capturedTexts());
                QString type = arg(typeI, regexp.capturedTexts());
                QString tooltip = arg(tooltipI, regexp.capturedTexts());
                QTextDocument text;
                text.setHtml(name);
                name = text.toPlainText();
                text.setHtml(type);
                type = text.toPlainText();
                text.setHtml(tooltip);
                tooltip = text.toPlainText();
                QVariantList vls;
                vls << arg(idI, regexp.capturedTexts());
                vls << type;
                vls << tooltip;
                ls->append(qMakePair(name, vls));
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
        if(!codec)
        {
            codec = QTextCodec::codecForName("utf8");
        }
        QString page = codec->toUnicode(reply->readAll());
        QRegExp regexp(config->getCurrentRemote().finishdeck);
        QString deckI = config->getCurrentRemote().deck;
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
        if(!codec)
        {
            codec = QTextCodec::codecForName("utf8");
        }
        QString page = codec->toUnicode(reply->readAll());
        QRegExp regexp(config->getCurrentRemote().finishname);
        regexp.setMinimal(true);
        regexp.indexIn(page);

        QString nameI = config->getCurrentRemote().name;

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
    connect(reply, static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
            this, &Remote::error);
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

