#include "deckmodel.h"
#include "range.h"
#include "configmanager.h"
#include "engine.h"
#include <algorithm>
#include <QDebug>

int DeckModel::counter = 0;

DeckModel::DeckModel(QObject *parent)
    : QObject(parent), m_timestamp(0), m_waiting(false), m_fresh(true), m_netwoking(make_networking())
{
    m_mainDeck = QSharedPointer<Type::DeckI>::create();
    m_extraDeck = QSharedPointer<Type::DeckI>::create();
    m_sideDeck = QSharedPointer<Type::DeckI>::create();
    m_id = ++counter;
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
    toCards(*m_mainDeck, snap.shot[0]);
    toCards(*m_extraDeck, snap.shot[1]);
    toCards(*m_sideDeck, snap.shot[2]);
    m_deckStatus = snap.deckStatus;
}

DeckModel::SnapShot DeckModel::currentSnapshot()
{
    SnapShot snap;
    toShot(snap.shot[0], *m_mainDeck);
    toShot(snap.shot[1], *m_extraDeck);
    toShot(snap.shot[2], *m_sideDeck);
    snap.deckStatus = m_deckStatus;
    return std::move(snap);
}

void DeckModel::makeSnapShot(bool mod)
{
    if(m_snapshots.size() > 50)
    {
        m_snapshots.pop_back();
    }
    m_redoSnapshots.clear();
    m_snapshots.push_front(currentSnapshot());

    m_fresh = false;
    if(mod)
    {
        m_deckStatus.modified = true;
    }
}

void DeckModel::undo()
{
    if(m_snapshots.size() >= 1)
    {
        if(m_redoSnapshots.size() > 50)
        {
            m_redoSnapshots.pop_back();
        }

        m_redoSnapshots.push_front(currentSnapshot());
        restoreSnapshot(m_snapshots.front());
        m_snapshots.pop_front();
    }
}

void DeckModel::redo()
{
    if(m_redoSnapshots.size() >= 1)
    {
        if(m_snapshots.size() > 50)
        {
            m_snapshots.pop_back();
        }
        m_snapshots.push_front(currentSnapshot());
        restoreSnapshot(m_redoSnapshots.front());
        m_redoSnapshots.pop_front();
    }
}

void DeckModel::clear()
{
    makeSnapShot();
    m_mainDeck->clear();
    m_extraDeck->clear();
    m_sideDeck->clear();
}

void DeckModel::sort()
{
    makeSnapShot();
    qSort(m_mainDeck->begin(), m_mainDeck->end(), itemCompare);
    qSort(m_extraDeck->begin(), m_extraDeck->end(), itemCompare);
    qSort(m_sideDeck->begin(), m_sideDeck->end(), itemCompare);
}

void DeckModel::shuffle()
{
    makeSnapShot();
    std::random_shuffle(m_mainDeck->begin(), m_mainDeck->end());
}

void DeckModel::abort()
{
    m_waiting = false;
    m_netwoking->abort();
}

void DeckModel::loadRemoteDeck(QString _id, QString name)
{
    if(m_waiting)
    {
        return;
    }
    m_waiting = true;
    emit ready(m_id, false);

    disconnect(m_netwoking.get());
    connect(m_netwoking.get(), &NetWorking::deck, this, [this, name](QString deck)
    {
        loadDeckInternal(deck, name, false);
    }, Qt::QueuedConnection);

    m_netwoking->getDeck(_id);
}

QString DeckModel::status()
{

    QString stat;
    stat += "[" + (m_deckStatus.isLocal ? ConfigManager::inst().getStr("label", "local", "本地")
                                      : ConfigManager::inst().getStr("label", "temp", "临时")) + "]";
    stat += m_deckStatus.name;
    stat += m_deckStatus.modified ? ("[" + ConfigManager::inst().getStr("label", "modified", "已修改") + "]") : "";
    return stat;
}

void DeckModel::loadDeck(QString lines, QString _name, bool local)
{
    if(m_waiting)
    {
        return;
    }
    loadDeckInternal(lines, _name, local);
}

void DeckModel::loadDeckInternal(QString lines, QString _name, bool local)
{

    int ts = ++m_timestamp;
    makeSnapShot(false);

    m_mainDeck->clear();
    m_extraDeck->clear();
    m_sideDeck->clear();


    m_deckStatus.name = _name;
    m_deckStatus.isLocal = local;
    m_deckStatus.modified = false;

    m_waiting = true;
    emit ready(m_id, false);
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
        foreach(auto &item, *m_mainDeck)
        {
            out << item.getId() << "\n";
        }
        out << "#extra\n";
        foreach(auto &item, *m_extraDeck)
        {
            out << item.getId() << "\n";
        }
        out << "!side\n";
        foreach(auto &item, *m_sideDeck)
        {
            out << item.getId() << "\n";
        }
        file.close();
        m_deckStatus.modified = false;
        m_deckStatus.isLocal = true;
        m_deckStatus.name = QFileInfo(file).completeBaseName();
    }
}

void DeckModel::loadFinished(int ts, ItemThread::Deck deck)
{
    m_waiting = false;
    if(ts == m_timestamp)
    {
        m_mainDeck->swap((*deck)[0]);
        m_extraDeck->swap((*deck)[1]);
        m_sideDeck->swap((*deck)[2]);
        emit ready(m_id, true);
        emit refresh(m_id);
    }
}

ItemThread::ItemThread(int _ts, QString _lines, DeckModel *_model)
    : QThread(), ts(_ts), lines(_lines), model(_model)
{
    static auto t = qRegisterMetaType<ItemThread::Deck>();
    Q_UNUSED(t);
}

optional<Card*> ItemThread::loadNewCard(quint32 id)
{
    auto &map = CardManager::inst().m_changedMap;
    auto it = map.find(id);
    if(it != map.end())
    {
       return CardManager::inst().getCard(it.value());
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


    if(auto card = CardManager::inst().getNewCard(name, ConfigManager::inst().m_waitForPass))
    {
        map.insert(id, card.value()->id);
        return card;
    }
    else
    {
        return nullopt;
    }
}

void ItemThread::run()
{
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
            if(ts != model->m_timestamp || !model->m_waiting)
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

                    optional<Card*> ocard;
                    if(ok)
                    {
                        ocard = CardManager::inst().getCard(id);
                        if(!ocard && ConfigManager::inst().m_convertPass && id >= 10000)
                        {
                            ocard = loadNewCard(id);
                        }
                    }
                    else
                    {
                        ocard = CardManager::inst().getNewCard(line, ConfigManager::inst().m_waitForPass);
                    }

                    if(ts != model->m_timestamp || !model->m_waiting)
                    {
                        return;
                    }

                    if(ocard)
                    {
                        Card &card = **ocard;
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
                    }
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
    Sdestroy_thread();
}
