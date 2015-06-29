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

class LimitCards{
public:
    LimitCards();
    QSharedPointer<QPixmap> limits[3];

    typedef QHash<int, int> Table;
    QList<QPair<QString, QSharedPointer<Table> > > tables;

    static void load();
    static QSharedPointer<QPixmap> getPixmap(int);

    static auto getTables() -> decltype(std::ref(tables))
    {
        return lim->tables;
    }

    static int getLimit(int id);

    static QSharedPointer<QVector<int> > getCards(int);
private:
    static QScopedPointer<LimitCards> lim;
};

#endif // LIMITS_H
