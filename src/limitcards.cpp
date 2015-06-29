#include "limitcards.h"
#include "card.h"
#include "config.h"
#include <QDebug>

QScopedPointer<LimitCards> LimitCards::lim;

LimitCards::LimitCards()
{
    QImage img;
    img.load("textures/lim.png");

    int w = img.width() / 2, h = img.height() / 2;

    for(auto &p : limits)
    {
        p = QSharedPointer<QPixmap>::create();
    }

    if(img.width() > 0)
    {
        limits[0]->convertFromImage(img.copy(0, 0, w, h));
        limits[1]->convertFromImage(img.copy(w, 0, w, h));
        limits[2]->convertFromImage(img.copy(0, h, w, h));
    }

    QFile lflist("lflist.conf");
    if(lflist.open(QFile::Text | QFile::ReadOnly))
    {
        QTextStream in(&lflist);
        QString name("N/A");
        QScopedPointer<Table> table(new Table());

        for(QString line = in.readLine(); !line.isNull(); line = in.readLine())
        {
            if(line.length() < 1)
            {
                continue;
            }
            if(line[0] == '#')
            {
                continue;
            }
            if(line[0] == '!')
            {
                tables.append(qMakePair(name, QSharedPointer<Table>(table.take())));
                name = line.mid(1);
                name.trimmed();
                table.reset(new Table());
                table->reserve(150);
                continue;
            }

            if(!table)
            {
                continue;
            }

            int pos = line.indexOf(QChar(' '));
            if(pos > 0)
            {
                int id = line.left(pos).toInt();
                int numPos = line.indexOf(QChar(' '), pos + 1);
                int lim = line.mid(pos + 1, numPos - pos - 1).toInt();
                table->insert(id, lim);
            }
        }
    }
}

void LimitCards::load()
{

    lim.reset(new LimitCards());
}

QSharedPointer<QPixmap> LimitCards::getPixmap(int i)
{
    if(i >= 0 && i <= 2)
    {
        return lim->limits[i];
    }
    return QSharedPointer<QPixmap>(nullptr);
}

int LimitCards::getLimit(int id)
{
    if(config->limit < 0)
    {
        return INT_MAX;
    }
    else if(lim->tables.size() <= config->limit)
    {
        return 3;
    }
    auto table = lim->tables[config->limit].second;

    auto card = CardPool::getCard(id);
    if(card->alias != 0)
    {
        id = card->alias;
    }

    auto it = table->find(id);
    if(it == table->end())
    {
        return 3;
    }
    else
    {
        return it.value();
    }
}

QSharedPointer<QVector<int> > LimitCards::getCards(int index)
{
    auto cards = QSharedPointer<QVector<int> >::create();
    if(index >= 0 && index < lim->tables.size())
    {
        auto table = lim->tables[index].second;
        cards->reserve(table->size());

        for(auto it = table->begin(); it != table->end(); ++it)
        {
            cards->append(it.key());
        }

        qSort(cards->begin(), cards->end(), [&] (int id1, int id2) {
           auto it1 = table->find(id1);
           auto it2 = table->find(id2);

           if(it1.value() == it2.value())
           {
               auto card1 = CardPool::getCard(id1), card2 = CardPool::getCard(id2);
               if(card1 && card2)
               {
                   return (card1->type & 7) < (card2->type & 7);
               }
           }
           return it1.value() < it2.value();
        });
    }
    return cards;
}
