#ifndef LIMIT_H
#define LIMIT_H

#include <QTextStream>
#include <QPixmap>
#include <QImage>
#include <QHash>
#include <QFile>
#include <QList>
#include <QScopedPointer>
#include <QSharedPointer>
#include <functional>
#include "typing.h"

class LimitCards{
public:
    LimitCards();
    QSharedPointer<QPixmap> limits[3];

    typedef QHash<quint32, int> Table;
    QList<QPair<QString, Table> > tables;

    static void load();
    static QSharedPointer<QPixmap> getPixmap(int);

    static auto getTables() -> decltype((tables))
    {
        return lim->tables;
    }

    static int getLimit(quint32 id);

    static Type::DeckP getCards(int);
private:
    static QScopedPointer<LimitCards> lim;
};

#endif // LIMITS_H
