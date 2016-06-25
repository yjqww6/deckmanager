#ifndef CARDITEM_H
#define CARDITEM_H
#include <QPixmap>
#include <QSharedPointer>
#include <QWeakPointer>
#include <QHash>
#include <QPoint>
#include "common.h"

class CardItem
{
public:
    CardItem(quint32 _id, bool small = false);

    CardItem() : m_id(0), m_pixmap(QSharedPointer<QPixmap>::create()) {}

    CardItem(const CardItem &item) : m_id(item.getId()), m_pixmap(item.getPixmap()) {}

    quint32 getId() const
    {
        return m_id;
    }

    QSharedPointer<QPixmap> getPixmap() const
    {
        return m_pixmap;
    }

    QPoint getPos() const
    {
        return m_pos;
    }

    void setPos(const QPoint &p)
    {
        m_pos = p;
    }

private:
    using Cache = QHash<quint32, QWeakPointer<QPixmap>>;
    static Cache m_large, m_small;

    quint32 m_id;
    QPoint m_pos;
    QSharedPointer<QPixmap> m_pixmap;
};

bool itemCompare(CardItem &a, CardItem &b);
bool idCompare(quint32 a, quint32 b);
#endif // CARDITEM_H
