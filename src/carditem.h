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
    CardItem(int _id, bool small = false);

    CardItem() : id(0), pixmap(nullptr) {}

    CardItem(const CardItem &item) : id(item.getId()), pixmap(item.getPixmap()) {}

    int getId() const
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
    class Pool : public QHash<int, QWeakPointer<QPixmap>>
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

    int id;
    QPoint pos;
    QSharedPointer<QPixmap> pixmap;
};

#endif // CARDITEM_H
