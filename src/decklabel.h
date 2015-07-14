#ifndef DECKLABEL
#define DECKLABEL

#include <QLabel>
#include "typing.h"
#include "card.h"
#include "config.h"


class DeckSizeLabel : public QLabel
{
    Q_OBJECT
public:
    DeckSizeLabel(const QString &_name) : QLabel(), name(_name) {}
public slots:
    void changeSize(int size)
    {
        setText(name + tr(" : ") + QString::number(size));
        adjustSize();
        updateGeometry();
    }

private:
    QString name;
};


class MainDeckLabel : public QLabel
{
    Q_OBJECT
public:
    MainDeckLabel()
        : QLabel()
    {
        setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
public slots:
    void deckChanged(Type::DeckI &ls)
    {
        int mSum = 0, sSum = 0, tSum = 0;
        foreach(auto &item, ls)
        {
            call_with_ref([&](Card &card) {
                if(card.type & Const::TYPE_MONSTER)
                {
                    mSum++;
                }
                else if(card.type & Const::TYPE_SPELL)
                {
                    sSum++;
                }
                else if(card.type & Const::TYPE_TRAP)
                {
                    tSum++;
                }
            }, cardPool->getCard(item.getId()));
        }
        QString text = config->getStr("string", "TYPE_MONSTER", "怪兽") + ": " + QString::number(mSum);
        text = text + " " + config->getStr("string", "TYPE_SPELL", "魔法") + ": " + QString::number(sSum);
        text = text + " " + config->getStr("string", "TYPE_TRAP", "陷阱") + ": " + QString::number(tSum);
        setText(text);
    }
};

class ExtraDeckLabel : public QLabel
{
    Q_OBJECT
public:
    ExtraDeckLabel()
        : QLabel()
    {
        setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    }
public slots:
    void deckChanged(Type::DeckI &ls)
    {
        int fSum = 0, sSum = 0, xSum = 0;
        foreach(auto &item, ls)
        {
            call_with_ref([&](Card &card) {

                if(card.type & Const::TYPE_FUSION)
                {
                    fSum++;
                }
                else if(card.type & Const::TYPE_SYNCHRO)
                {
                    sSum++;
                }
                else if(card.type & Const::TYPE_XYZ)
                {
                    xSum++;
                }
            }, cardPool->getCard(item.getId()));
        }
        QString text = config->getStr("string", "TYPE_FUSION", "融合") + ": " + QString::number(fSum);
        text = text + " " + config->getStr("string", "TYPE_SYNCHRO", "同调") + ": " + QString::number(sSum);
        text = text + " " + config->getStr("string", "TYPE_XYZ", "XYZ") + ": " + QString::number(xSum);

        setText(text);
    }
};
#endif // DECKLABEL

