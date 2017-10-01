#ifndef DECKLABEL
#define DECKLABEL

#include <QLabel>
#include "types.h"
#include "card.h"
#include "configmanager.h"


class DeckSizeLabel : public QLabel
{
    Q_OBJECT
public:
    DeckSizeLabel(const QString &_name) : QLabel(), m_name(_name) {}
public slots:
    void changeSize(int size)
    {
        setText(m_name + " : " + QString::number(size));
        adjustSize();
        updateGeometry();
    }

private:
    QString m_name;
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
            if(auto ocard = CardManager::inst().getCard(item.getId()))
            {
                Card &card = **ocard;
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
            }
        }
        QString text = ConfigManager::inst().getStr("string", "TYPE_MONSTER", "怪兽") + ": " + QString::number(mSum);
        text = text + " " + ConfigManager::inst().getStr("string", "TYPE_SPELL", "魔法") + ": " + QString::number(sSum);
        text = text + " " + ConfigManager::inst().getStr("string", "TYPE_TRAP", "陷阱") + ": " + QString::number(tSum);
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
        int fSum = 0, sSum = 0, xSum = 0, lSum = 0;
        foreach(auto &item, ls)
        {
            if(auto ocard = CardManager::inst().getCard(item.getId()))
            {
                Card &card = **ocard;

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
                else if(card.type & Const::TYPE_LINK)
                {
                    lSum++;
                }
            }
        }
        QString text = ConfigManager::inst().getStr("string", "TYPE_FUSION", "融合") + ": " + QString::number(fSum);
        text = text + " " + ConfigManager::inst().getStr("string", "TYPE_SYNCHRO", "同调") + ": " + QString::number(sSum);
        text = text + " " + ConfigManager::inst().getStr("string", "TYPE_XYZ", "XYZ") + ": " + QString::number(xSum);
        text = text + " " + ConfigManager::inst().getStr("string", "TYPE_LINK", "LINK") + ": " + QString::number(lSum);

        setText(text);
    }
};
#endif // DECKLABEL

