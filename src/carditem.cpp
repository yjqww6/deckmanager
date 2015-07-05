#include "carditem.h"
#include "expansions.h"
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

