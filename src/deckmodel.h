#ifndef DECKMODEL_H
#define DECKMODEL_H
#include <QObject>
#include "types.h"
#include "card.h"
#include "networking.h"
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
    optional<Card*> loadNewCard(quint32 id);
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
        DeckStatus(const DeckStatus &other) = default;
        bool isLocal, modified, fresh;
        QString name;
    };

    struct SnapShot
    {
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

    void loadRemoteDeck(QString m_id, QString name);
    void loadDeck(QString lines, QString _name, bool local);
    void loadDeckInternal(QString lines, QString _name, bool local);
    void saveDeck(QString path);
    QString status();
signals:
    void ready(int, bool);
    void refresh(int);
public slots:
    void loadFinished(int ts, ItemThread::Deck);

public:
    DeckStatus m_deckStatus;
    int m_id;
    QSharedPointer<Type::DeckI> m_mainDeck, m_extraDeck, m_sideDeck;
    QList<SnapShot> m_snapshots, m_redoSnapshots;
    QAtomicInt m_timestamp;
    bool m_waiting;
    bool m_fresh;
    std::shared_ptr<NetWorking> m_netwoking;

    static int counter;
};

#endif // DECKMODEL_H
