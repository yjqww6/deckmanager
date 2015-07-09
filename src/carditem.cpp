#include "carditem.h"
#include "expansions.h"
#include "card.h"
#include <QDebug>

CardItem::Pool CardItem::pool;
CardItem::Pool CardItem::spool;
QString CardItem::bigPics = "pics/";
QString CardItem::smallPics = "pics/thumbnail/";
QString CardItem::unknown = "textures/unknown.jpg";

static QSharedPointer<QPixmap> readPic(QString path)
{
    auto p = QSharedPointer<QPixmap>::create(path, "1");
    if(!p || p->width() == 0)
    {
        QByteArray arr = expansions->open(path);
        p->loadFromData(arr, "1");
    }
    return p;
}

CardItem::CardItem(quint32 _id, bool small)
    : id(_id)
{
    auto &thePool = small ? spool : pool;
    auto it = thePool.find(id);
    if(it == thePool.end() || it.value().isNull())
    {
        if(small)
        {
            QString path = smallPics + QString::number(id) + ".jpg";

            pixmap = readPic(path);

            if(pixmap->width() == 0)
            {
                path = bigPics + QString::number(id) + ".jpg";
                pixmap = readPic(path);
            }
        }
        else
        {
            QString path = bigPics + QString::number(id) + ".jpg";
            pixmap = readPic(path);


            if(pixmap->width() == 0)
            {
                path = smallPics + QString::number(id) + ".jpg";
                pixmap = readPic(path);
            }

        }

        if(pixmap->width() == 0)
        {
            QString path = unknown;
            pixmap = readPic(path);
        }

        thePool.insert(id, pixmap.toWeakRef());
    }
    else
    {
        pixmap = it.value().toStrongRef();
    }
}

bool idCompare(quint32 a, quint32 b)
{
    auto card1 = cardPool->getCard(a), card2 = cardPool->getCard(b);
    if(card1.isNull() || card2.isNull())
    {
        return a < b;
    }
    auto ca = card1.ref(), cb = card2.ref();
    int ta = ca.type & 7, tb = cb.type & 7;
    if(ta != tb)
    {
        return ta < tb;
    }
    else if(ca.type != cb.type)
    {
        return ca.type < cb.type;
    }
    else if(ca.type & Const::TYPE_MONSTER)
    {
        if(ca.level != cb.level)
        {
            return ca.level >= cb.level;
        }

        if(ca.atk != cb.atk)
        {
            return ca.atk >= cb.atk;
        }
        if(ca.def != cb.def)
        {
            return ca.def >= cb.def;
        }
        return ca.id < cb.id;
    }
    else
    {
        return ca.id < cb.id;
    }
}


bool itemCompare(CardItem &a, CardItem &b)
{
    return idCompare(a.getId(), b.getId());
}
