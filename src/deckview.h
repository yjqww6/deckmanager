#ifndef DECKVIEW_H
#define DECKVIEW_H

#include <QScrollArea>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextStream>
#include <QSharedPointer>
#include <QEventLoop>
#include <QCheckBox>
#include <QMessageBox>
#include <QFileInfo>
#include <QAction>
#include <QToolBar>

#include "deckwidget.h"
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


class DeckView;

class ItemThread : public QThread
{
    Q_OBJECT
public:
    typedef QSharedPointer<QList<QList<CardItem> > > Deck;
    ItemThread(int _load, QString _lines, DeckView *parent);
    void run();
    int load;
    Deck::value_type deck;
    QString lines;
    DeckView *parent;
    QSharedPointer<Card> loadNewCard(quint32 id);
signals:
    void finishLoad(int, ItemThread::Deck);
};

Q_DECLARE_METATYPE(ItemThread::Deck)

class DeckView : public QWidget
{
    Q_OBJECT
public:
    friend class ItemThread;

    DeckView(QWidget *parent);
    DeckWidget *getMain()
    {
        return mainDeck;
    }

    DeckWidget *getExtra()
    {
        return extraDeck;
    }

    DeckWidget *getSide()
    {
        return sideDeck;
    }

    void resizeEvent(QResizeEvent *event);
signals:
    void currentIdChanged(int);
    void clickId(int);
    void details(int);
    void save();
    void statusChanged(QString);
    void refreshLocals();

public slots:

    void loadDeck(QString, QString, bool);
    void saveDeck(QString);
    void saveSlot();

    void clearDeck();

    void setCurrentCardId(quint32 id)
    {
        mainDeck->setCurrentCardId(id);
        extraDeck->setCurrentCardId(id);
        sideDeck->setCurrentCardId(id);
    }

    void idClicked(quint32 id)
    {
        emit clickId(id);
    }

    void makeSnapshot(bool mod = true);
    void undo();
    void redo();
    void newDeck();
    void deleteDeck();

    void setStatus()
    {
        QString stat = "deckmanager - by qww6 ";
        stat += "[" + (deckStatus.isLocal ? config->getStr("label", "local", "本地")
                                          : config->getStr("label", "temp", "临时")) + "]";
        stat += deckStatus.name;
        stat += deckStatus.modified ? ("[" + config->getStr("label", "modified", "已修改") + "]") : "";
        emit statusChanged(stat);
    }

    void loadFinished(int load, ItemThread::Deck);
    void abort()
    {
        waiting = false;
        setReady(true);
    }

    void checkLeave()
    {
        mainDeck->checkLeave();
        extraDeck->checkLeave();
        sideDeck->checkLeave();
    }

private slots:

    void loadOtherNames()
    {
        CardPool::loadOtherNames();
    }

    void setReady(bool t)
    {
        abortAction->setDisabled(t);
    }

    void help();
    void sort();
    void home();
    void print();
    void hideSide();

private:
    class DeckStatus
    {
    public:
        DeckStatus() : isLocal(false), modified(false) {}
        DeckStatus(const DeckStatus &other)
            : isLocal(other.isLocal), modified(other.modified), name(other.name)
        {}
        bool isLocal, modified;
        QString name;
    } deckStatus;

    class SnapShot
    {
    public:
        SnapShot() {}
        SnapShot(SnapShot&& other)
        {
            shot[0] = std::move(other.shot[0]);
            shot[1] = std::move(other.shot[1]);
            shot[2] = std::move(other.shot[2]);
            deckStatus = other.deckStatus;
        }

        SnapShot(const SnapShot& other)
        {
            shot[0] = other.shot[0];
            shot[1] = other.shot[1];
            shot[2] = other.shot[2];
            deckStatus = other.deckStatus;
        }

        SnapShot& operator=(const SnapShot& other)
        {
            shot[0] = other.shot[0];
            shot[1] = other.shot[1];
            shot[2] = other.shot[2];
            deckStatus = other.deckStatus;
            return *this;
        }

        Type::Deck shot[3];
        DeckStatus deckStatus;
    };

    void updateButtons();

    QList<SnapShot> snapshots, redoSnapshots;
    SnapShot currentSnapshot();
    void restoreSnapshot(SnapShot&);

    DeckWidget *mainDeck;
    DeckWidget *extraDeck;
    DeckWidget *sideDeck;
    DeckSizeLabel *st;
    QAction *undoAction, *redoAction;
    QAction *abortAction;
    QToolBar *toolbar;
    int currentLoad;
    QHash<quint32, quint32> map;
    bool waiting;
    bool sideHidden;
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
    void deckChanged(QList<CardItem> &ls)
    {
        int mSum = 0, sSum = 0, tSum = 0;
        foreach(auto &item, ls)
        {
            auto card = CardPool::getCard(item.getId());
            if(card->type & Card::TYPE_MONSTER)
            {
                mSum++;
            }
            else if(card->type & Card::TYPE_SPELL)
            {
                sSum++;
            }
            else if(card->type & Card::TYPE_TRAP)
            {
                tSum++;
            }
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
    void deckChanged(QList<CardItem> &ls)
    {
        int fSum = 0, sSum = 0, xSum = 0;
        foreach(auto &item, ls)
        {
            auto card = CardPool::getCard(item.getId());
            if(card->type & Card::TYPE_FUSION)
            {
                fSum++;
            }
            else if(card->type & Card::TYPE_SYNCHRO)
            {
                sSum++;
            }
            else if(card->type & Card::TYPE_XYZ)
            {
                xSum++;
            }
        }
        QString text = config->getStr("string", "TYPE_FUSION", "融合") + ": " + QString::number(fSum);
        text = text + " " + config->getStr("string", "TYPE_SYNCHRO", "同调") + ": " + QString::number(sSum);
        text = text + " " + config->getStr("string", "TYPE_XYZ", "XYZ") + ": " + QString::number(xSum);

        setText(text);
    }
};


#endif // DECKVIEW_H
