#ifndef CARD_H
#define CARD_H

#include <QString>
#include <QHash>
#include <QFile>
#include <QThread>
#include "constant.h"
#include <experimental/optional>
#include <QDebug>
#include <QMutex>
#include <memory>
#include <unordered_map>
#include "common.h"

using std::experimental::optional;
using std::experimental::nullopt;

struct Card
{
    bool inExtra()
    {
        return (type & (Const::TYPE_XYZ | Const::TYPE_SYNCHRO | Const::TYPE_FUSION)) != 0;
    }

    quint32 id;
    int     ot;
    quint32 alias;
    quint64 setcode;
    quint32 type;
    int     atk;
    int     def;
    quint32 scale;
    quint32 level;
    quint32 race;
    quint32 attribute;
    quint32 category;
    QString name;
    QString effect;
};

class CardManager : public enable_singleton<CardManager>
{
public:
    std::unordered_map<quint32, std::unique_ptr<Card>> m_cards;

    QHash<QString, quint32> m_newPool;
    QHash<quint32, quint32> m_changedMap;
    QHash<quint32, QString> m_races, m_types, m_attrs;

    void loadCardData(QStringList paths);

    optional<Card*> getCard(quint32 id);
    optional<Card*> getNewCard(QString name, bool wait = true);

    QString getType(quint32);
    QString getRace(quint32);
    QString getAttr(quint32);
};

#endif // CARD_H
