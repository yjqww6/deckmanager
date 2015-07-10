#ifndef LIMIT_H
#define LIMIT_H

#include <QTextStream>
#include <QPixmap>
#include <QImage>
#include <QHash>
#include <QFile>
#include <QList>
#include <QSharedPointer>
#include <functional>
#include "typing.h"

class LimitCards
{
public:
    LimitCards();
    QSharedPointer<QPixmap> limits[3];

    typedef QHash<quint32, int> Table;
    QList<QPair<QString, Table> > tables;

    QSharedPointer<QPixmap> getPixmap(int);

    auto getTables() -> decltype((tables))
    {
        return tables;
    }

    int getLimit(quint32 id);

    Type::DeckP getCards(int);
};

extern LimitCards *limitCards;
#endif // LIMITS_H
