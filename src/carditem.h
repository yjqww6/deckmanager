#ifndef CARDITEM_H
#define CARDITEM_H
#include <QPixmap>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QHash>
#include <QPoint>
#include <QFile>

class CardItem
{
public:
    CardItem(quint32 _id, bool small = false);

    CardItem() : id(0), pixmap(nullptr) {}

    CardItem(const CardItem &item) : id(item.getId()), pixmap(item.getPixmap()) {}

    quint32 getId() const
    {
        return id;
    }

    QSharedPointer<QPixmap> getPixmap() const
    {
        return pixmap;
    }

    QPoint getPos()
    {
        return pos;
    }

    void setPos(const QPoint &p)
    {
        pos = p;
    }

private:
    class Pool : public QHash<quint32, QWeakPointer<QPixmap>>
    {
    public:
        Pool()
        {
            reserve(10000);
        }
    };
    static Pool pool;
    static Pool spool;
    static QString bigPics;
    static QString smallPics;
    static QString unknown;

    quint32 id;
    QPoint pos;
    QSharedPointer<QPixmap> pixmap;
};

bool itemCompare(CardItem &a, CardItem &b);
bool idCompare(quint32 a, quint32 b);
#endif // CARDITEM_H
