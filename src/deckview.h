#ifndef DECKVIEW_H
#define DECKVIEW_H

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextStream>
#include <QSharedPointer>
#include <QEventLoop>
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
    typedef QSharedPointer<QList<Type::DeckI> > Deck;
    ItemThread(int, QString _lines, DeckView *parent);
    void run();
    int ts;
    Deck::value_type deck;
    QString lines;
    DeckView *parent;
    Wrapper<Card> loadNewCard(quint32 id);
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

    Type::DeckP getDeck()
    {
        auto deck = Type::DeckP::create();
        foreach(auto &item, mainDeck->getDeck())
        {
            deck->append(item.getId());
        }
        foreach(auto &item, extraDeck->getDeck())
        {
            deck->append(item.getId());
        }
        if(!sideHidden)
        {
            foreach(auto &item, sideDeck->getDeck())
            {
                deck->append(item.getId());
            }
        }
        return deck;
    }

    void resizeEvent(QResizeEvent *event);
signals:
    void currentIdChanged(int);
    void clickId(int);
    void details(int);
    void save(QString);
    void statusChanged(QString);
    void refreshLocals();
    void deckText(Type::DeckI&, Type::DeckI&, Type::DeckI&, int);

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
    int timestamp;
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


#endif // DECKVIEW_H
