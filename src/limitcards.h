#ifndef LIMIT_H
#define LIMIT_H

#include <QTextStream>
#include <QPixmap>
#include <QImage>
#include <QHash>
#include <QFile>
#include <QList>
#include <QScopedPointer>
#include <functional>
#include "types.h"
#include "common.h"

class LimitCards : public enable_singleton<LimitCards>
{
public:
    LimitCards();
    void load();

    QPixmap *getPixmap(int);
    int getLimit(quint32 id);

    Type::DeckP getCards(int);
public:

    QScopedPointer<QPixmap> m_limits[3];

    typedef QHash<quint32, int> Table;
    QList<QPair<QString, Table>> m_tables;
};
#endif // LIMITS_H
