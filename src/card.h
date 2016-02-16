#ifndef CARD_H
#define CARD_H

#include <QString>
#include <QHash>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QFile>
#include <QThread>
#include "constant.h"
#include "wrapper.h"
#include <QDebug>
#include <memory>
#include <unordered_map>

struct Card
{
    bool inExtra()
    {
        return (type & (Const::TYPE_XYZ | Const::TYPE_SYNCHRO | Const::TYPE_FUSION)) != 0;
    }

    quint32 id;
    int ot;
    quint32 alias;
    quint64 setcode;
    quint32 type;
    int atk;
    int def;
    quint32 scale;
    quint32 level;
    quint32 race;
    quint32 attribute;
    quint32 category;
    QString name;
    QString effect;



    QString cardType();

    QString cardRace();

    QString cardAttr();

    QString cardAD()
    {
        return adToString(atk) + "/" + adToString(def);
    }

    Card() {}

    static QString adToString(int ad)
    {
        if(ad == -2)
        {
            return "?";
        }
        else
        {
            return QString::number(ad);
        }
    }
};

class CardPool;

class LoadThread : public QThread
{
    Q_OBJECT
public:
    LoadThread(QObject *parent, CardPool *thePool);
    void run();

private:
    CardPool *thePool;
};


class CardPool
{
private:
    QStringList cdbPath;
    void loadCard(QSqlQuery&);
    std::unordered_map<quint32, std::unique_ptr<Card> > pool;
    QHash<QString, quint32> newPool;
    QHash<quint32, quint32> changedMap;
    QHash<quint32, QString> races;
    QHash<quint32, QString> types;
    QHash<quint32, QString> attrs;
    bool otherNamesDone;
    bool acc;
    LoadThread loadThread;

public:
    friend class LoadThread;

    CardPool(QStringList paths);

    Wrapper<Card> getCard(quint32 id);
    Wrapper<Card> getNewCard(QString name, bool wait = true);

    void loadNames();
    auto getTypes() -> decltype((types))
    {
        return types;
    }

    auto getRaces() -> decltype((races))
    {
        return races;
    }
    auto getAttrs() -> decltype((attrs))
    {
        return attrs;
    }

    auto getPool() -> decltype((pool))
    {
        return pool;
    }

    auto getMap() -> decltype((changedMap))
    {
        return changedMap;
    }

    LoadThread *getThread()
    {
        return &loadThread;
    }

    QString getType(quint32);
    QString getRace(quint32);
    QString getAttr(quint32);
};

extern CardPool *cardPool;

#endif // CARD_H
