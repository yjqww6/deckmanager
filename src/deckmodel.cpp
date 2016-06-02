#include "deckmodel.h"
#include "range.h"
#include "config.h"
#include "engine.h"
#include <algorithm>
#include <QDebug>

int DeckModel::counter = 0;

DeckModel::DeckModel(QObject *parent)
    : QObject(parent), timestamp(0), waiting(false), fresh(true), netwoking(make_networking())
{
    mainDeck = QSharedPointer<Type::DeckI>::create();
    extraDeck = QSharedPointer<Type::DeckI>::create();
    sideDeck = QSharedPointer<Type::DeckI>::create();
    id = ++counter;
}

static void toShot(Type::Deck &shot, Type::DeckI& items)
{
    shot.reserve(items.size());
    foreach(auto &item, items)
    {
        shot.append(item.getId());
    }
}

static void toCards(Type::DeckI& items, Type::Deck &shot)
{
    Type::DeckI temp;
    temp.reserve(shot.size());
    foreach(auto id, shot)
    {
        temp.append(CardItem(id));
    }
    items.swap(temp);
}

void DeckModel::restoreSnapshot(SnapShot &snap)
{
    toCards(*mainDeck, snap.shot[0]);
    toCards(*extraDeck, snap.shot[1]);
    toCards(*sideDeck, snap.shot[2]);
    deckStatus = snap.deckStatus;
}

DeckModel::SnapShot DeckModel::currentSnapshot()
{
    SnapShot snap;
    toShot(snap.shot[0], *mainDeck);
    toShot(snap.shot[1], *extraDeck);
    toShot(snap.shot[2], *sideDeck);
    snap.deckStatus = deckStatus;
    return std::move(snap);
}

void DeckModel::makeSnapShot(bool mod)
{
    if(snapshots.size() > 50)
    {
        snapshots.pop_back();
    }
    redoSnapshots.clear();
    snapshots.push_front(currentSnapshot());

    fresh = false;
    if(mod)
    {
        deckStatus.modified = true;
    }
}

void DeckModel::undo()
{
    if(snapshots.size() >= 1)
    {
        if(redoSnapshots.size() > 50)
        {
            redoSnapshots.pop_back();
        }

        redoSnapshots.push_front(currentSnapshot());
        restoreSnapshot(snapshots.front());
        snapshots.pop_front();
    }
}

void DeckModel::redo()
{
    if(redoSnapshots.size() >= 1)
    {
        if(snapshots.size() > 50)
        {
            snapshots.pop_back();
        }
        snapshots.push_front(currentSnapshot());
        restoreSnapshot(redoSnapshots.front());
        redoSnapshots.pop_front();
    }
}

void DeckModel::clear()
{
    makeSnapShot();
    mainDeck->clear();
    extraDeck->clear();
    sideDeck->clear();
}

void DeckModel::sort()
{
    makeSnapShot();
    qSort(mainDeck->begin(), mainDeck->end(), itemCompare);
    qSort(extraDeck->begin(), extraDeck->end(), itemCompare);
    qSort(sideDeck->begin(), sideDeck->end(), itemCompare);
}

void DeckModel::shuffle()
{
    makeSnapShot();
    std::random_shuffle(mainDeck->begin(), mainDeck->end());
}

void DeckModel::abort()
{
    waiting = false;
    netwoking->abort();
}

void DeckModel::loadRemoteDeck(QString _id, QString name)
{
    if(waiting)
    {
        return;
    }
    waiting = true;
    emit ready(id, false);

    disconnect(netwoking.get());
    connect(netwoking.get(), &NetWorking::deck, this, [this, name](QString deck)
    {
        loadDeckInternal(deck, name, false);
    }, Qt::QueuedConnection);

    netwoking->getDeck(_id);
}

QString DeckModel::status()
{

    QString stat;
    stat += "[" + (deckStatus.isLocal ? config->getStr("label", "local", "本地")
                                      : config->getStr("label", "temp", "临时")) + "]";
    stat += deckStatus.name;
    stat += deckStatus.modified ? ("[" + config->getStr("label", "modified", "已修改") + "]") : "";
    return stat;
}

void DeckModel::loadDeck(QString lines, QString _name, bool local)
{
    if(waiting)
    {
        return;
    }
    loadDeckInternal(lines, _name, local);
}

void DeckModel::loadDeckInternal(QString lines, QString _name, bool local)
{

    int ts = ++timestamp;
    makeSnapShot(false);

    mainDeck->clear();
    extraDeck->clear();
    sideDeck->clear();


    deckStatus.name = _name;
    deckStatus.isLocal = local;
    deckStatus.modified = false;

    waiting = true;
    emit ready(id, false);
    auto thread = new ItemThread(ts, lines, this);
    connect(thread, &ItemThread::finishLoad, this, &DeckModel::loadFinished);
    connect(thread, &ItemThread::finished, thread, &ItemThread::deleteLater);
    connect(this, &DeckModel::destroyed, thread, &ItemThread::quit);
    thread->start();
}

void DeckModel::saveDeck(QString path)
{
    QFile file(path);
    if(file.open(QFile::WriteOnly | QFile::Text))
    {
        QTextStream out(&file);
        out << "#create by ...\n#main\n";
        foreach(auto &item, *mainDeck)
        {
            out << item.getId() << "\n";
        }
        out << "#extra\n";
        foreach(auto &item, *extraDeck)
        {
            out << item.getId() << "\n";
        }
        out << "!side\n";
        foreach(auto &item, *sideDeck)
        {
            out << item.getId() << "\n";
        }
        file.close();
        deckStatus.modified = false;
        deckStatus.isLocal = true;
        deckStatus.name = QFileInfo(file).completeBaseName();
    }
}

void DeckModel::loadFinished(int ts, ItemThread::Deck deck)
{
    waiting = false;
    if(ts == timestamp)
    {
        mainDeck->swap((*deck)[0]);
        extraDeck->swap((*deck)[1]);
        sideDeck->swap((*deck)[2]);
        emit ready(id, true);
        emit refresh(id);
    }
}

ItemThread::ItemThread(int _ts, QString _lines, DeckModel *_model)
    : QThread(), ts(_ts), lines(_lines), model(_model)
{
    static auto t = qRegisterMetaType<ItemThread::Deck>();
    Q_UNUSED(t);
}

Wrapper<Card> ItemThread::loadNewCard(quint32 id)
{
    auto &map = cardPool->changedMap;
    auto it = map.find(id);
    if(it != map.end())
    {
       return cardPool->getCard(it.value());
    }

    QEventLoop loop;
    QString name;
    auto networking = make_networking();
    loop.connect(networking.get(), &NetWorking::name, [&](QString text) {
        name = text;
    });

    loop.connect(networking.get(), &NetWorking::ready, this, [&](bool ready)
    {
        if(ready)
        {
            loop.quit();
        }
    }, Qt::QueuedConnection);

    networking->getName(id);
    loop.exec();

    auto card = cardPool->getNewCard(name, config->waitForPass);
    call_with_ref([&](Card &card) {
        map.insert(id, card.id);
    }, card.copy());
    return card;
}

void ItemThread::run()
{
    SchemeThreadActivator act;
    try
    {

        QTextStream in(&lines);
        bool side = false;
        for(auto i: range(3))
        {
            Q_UNUSED(i);
            decltype(deck)::value_type temp;
            deck.append(std::move(temp));
        }

        for(QString line = in.readLine(); !line.isNull(); line = in.readLine())
        {
            if(ts != model->timestamp || !model->waiting)
            {
                return;
            }
            if(line.length() > 0)
            {
                if(line[0] == '#')
                {
                    continue;
                }
                else if(line[0] == '!')
                {
                    side = true;
                    continue;
                }
                else
                {
                    bool ok = true;
                    quint32 id = line.toUInt(&ok);

                    Wrapper<Card> card;
                    if(ok)
                    {
                        card = cardPool->getCard(id);
                        if(card.isNull() && config->convertPass && id >= 10000)
                        {
                            card = loadNewCard(id);
                        }
                    }
                    else
                    {
                        card = cardPool->getNewCard(line, config->waitForPass);
                    }

                    if(ts != model->timestamp || !model->waiting)
                    {
                        return;
                    }

                    call_with_ref([&](Card &card) {
                        id = card.id;
                        if(side)
                        {
                            deck[2].append(CardItem(id));
                        }
                        else
                        {
                            if(card.inExtra())
                            {
                                deck[1].append(CardItem(id));
                            }
                            else
                            {
                                deck[0].append(CardItem(id));
                            }
                        }
                    }, std::move(card));
                }
            }
        }

        auto ptr = Deck::create();
        ptr->swap(deck);
        emit finishLoad(ts, ptr);
    }
    catch(...)
    {

    }
}
