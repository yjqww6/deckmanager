#ifndef DECKMODEL_H
#define DECKMODEL_H
#include <QObject>
#include "typing.h"
#include "card.h"
#include "remote.h"
#include <QAtomicInt>
#include <functional>


class DeckModel;


class ItemThread : public QThread
{
    Q_OBJECT
public:
    typedef QSharedPointer<QList<Type::DeckI> > Deck;
    ItemThread(int, QString _lines, DeckModel *model);
    void run();
    int ts;
    Deck::value_type deck;
    QString lines;
    DeckModel *model;
    Wrapper<Card> loadNewCard(quint32 id);
signals:
    void finishLoad(int, ItemThread::Deck);
};


Q_DECLARE_METATYPE(ItemThread::Deck)

class DeckModel : public QObject
{
    Q_OBJECT
public:
    explicit DeckModel(QObject *parent = 0);

    class DeckStatus
    {
    public:
        DeckStatus() : isLocal(false), modified(false), fresh(false) {}
        DeckStatus(const DeckStatus &other)
            : isLocal(other.isLocal), modified(other.modified), fresh(other.fresh), name(other.name)
        {}
        bool isLocal, modified, fresh;
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


    SnapShot currentSnapshot();
    void restoreSnapshot(SnapShot&);
    void makeSnapShot(bool mod = true);
    void undo();
    void redo();
    void clear();
    void sort();
    void shuffle();
    void abort();

    void loadRemoteDeck(QString id, QString name);
    void loadDeck(QString lines, QString _name, bool local);
    void loadDeckInternal(QString lines, QString _name, bool local);
    void saveDeck(QString path);
    QString status();

    int id;
    QSharedPointer<Type::DeckI> mainDeck, extraDeck, sideDeck;
    QList<SnapShot> snapshots, redoSnapshots;
    QAtomicInt timestamp;
    bool waiting;
    bool fresh;
    Remote remote;

    static int counter;
signals:
    void ready(int, bool);
    void refresh(int);
public slots:
    void loadFinished(int ts, ItemThread::Deck);
};

#endif // DECKMODEL_H
