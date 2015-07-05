#include "card.h"
#include "expansions.h"
#include "config.h"
#include "range.h"
#include <QDebug>

QScopedPointer<CardPool> CardPool::thePool;

void CardPool::loadCard(quint32 id)
{
    auto it = pool.find(id);
    if(it == pool.end() || it->isNull())
    {
        auto card = make_card(id);
        if(card)
        {
            pool.insert(id, card);
        }
        return;
    }
    return;
}

QSharedPointer<Card> CardPool::findCard(quint32 id)
{
    auto it = pool.find(id);
    if(it == pool.end())
    {
        return QSharedPointer<Card>(nullptr);
    }
    return it.value();
}

QSharedPointer<Card> CardPool::make_card(quint32 id)
{
    auto card = QSharedPointer<Card>(new Card);
    QSqlQuery query;
    query.prepare("select * from datas where id = :id");
    query.bindValue(":id", id);
    query.exec();

    if(!query.first())
    {
        return QSharedPointer<Card>(nullptr);
    }

#define ASSIGN(field, index) card->field = query.value(index).toUInt();
    ASSIGN(id, 0);
    ASSIGN(ot, 1);
    ASSIGN(alias, 2);
    card->setcode = query.value(3).toULongLong();
    ASSIGN(type, 4);
    card->atk = query.value(5).toInt();
    card->def = query.value(6).toInt();
    ASSIGN(level, 7);
    ASSIGN(race, 8);
    ASSIGN(attribute, 9);
    ASSIGN(category, 10);
    card->scale = (card->level & 0xff0000) >> 16;
    card->level = card->level & 0xffff;

    query.prepare("select * from texts where id = :id");
    query.bindValue(":id", id);
    query.exec();

    if(!query.first())
    {
        return card;
    }
    card->name = query.value(1).toString();
    card->effect = query.value(2).toString();
    return card;
}

CardPool::CardPool()
{
#define INSERT(container, field, str) \
    do { \
        auto &cont = container;\
        auto name = config->getStr("string", #field, str);\
        cont.insert(Card::field, name);\
    } while(false)

    INSERT(types, TYPE_MONSTER, "怪兽");
    INSERT(types, TYPE_SPELL, "魔法");
    INSERT(types, TYPE_TRAP, "陷阱");
    INSERT(types, TYPE_NORMAL, "通常");
    INSERT(types, TYPE_EFFECT, "效果");
    INSERT(types, TYPE_FUSION, "融合");
    INSERT(types, TYPE_RITUAL, "仪式");
    INSERT(types, TYPE_TRAPMONSTER, "陷阱怪兽");
    INSERT(types, TYPE_SPIRIT, "灵魂");
    INSERT(types, TYPE_UNION, "同盟");
    INSERT(types, TYPE_DUAL, "二重");
    INSERT(types, TYPE_TUNER, "调整");
    INSERT(types, TYPE_SYNCHRO, "同调");
    INSERT(types, TYPE_TOKEN, "衍生物");
    INSERT(types, TYPE_QUICKPLAY, "速攻");
    INSERT(types, TYPE_CONTINUOUS, "永续");
    INSERT(types, TYPE_EQUIP, "装备");
    INSERT(types, TYPE_FIELD, "场地");
    INSERT(types, TYPE_COUNTER, "反击");
    INSERT(types, TYPE_FLIP, "反转");
    INSERT(types, TYPE_TOON, "卡通");
    INSERT(types, TYPE_XYZ, "XYZ");
    INSERT(types, TYPE_PENDULUM, "灵摆");
    INSERT(races, RACE_WARRIOR, "战士");
    INSERT(races, RACE_SPELLCASTER, "魔法师");
    INSERT(races, RACE_FAIRY, "天使");
    INSERT(races, RACE_FIEND, "恶魔");
    INSERT(races, RACE_ZOMBIE, "不死");
    INSERT(races, RACE_MACHINE, "机械");
    INSERT(races, RACE_AQUA, "水");
    INSERT(races, RACE_PYRO, "炎");
    INSERT(races, RACE_ROCK, "岩石");
    INSERT(races, RACE_WINDBEAST, "鸟兽");
    INSERT(races, RACE_PLANT, "植物");
    INSERT(races, RACE_INSECT, "昆虫");
    INSERT(races, RACE_THUNDER, "雷");
    INSERT(races, RACE_DRAGON, "龙");
    INSERT(races, RACE_BEAST, "兽");
    INSERT(races, RACE_BEASTWARRIOR, "兽战士");
    INSERT(races, RACE_DINOSAUR, "恐龙");
    INSERT(races, RACE_FISH, "鱼");
    INSERT(races, RACE_SEASERPENT, "海龙");
    INSERT(races, RACE_REPTILE, "爬虫");
    INSERT(races, RACE_PSYCHO, "念动力");
    INSERT(races, RACE_DEVINE, "幻神兽");
    INSERT(races, RACE_CREATORGOD, "创造神");
    INSERT(races, RACE_PHANTOMDRAGON, "幻龙");


    INSERT(attrs, ATTRIBUTE_DARK, "暗");
    INSERT(attrs, ATTRIBUTE_LIGHT, "光");
    INSERT(attrs, ATTRIBUTE_WATER, "水");
    INSERT(attrs, ATTRIBUTE_FIRE, "火");
    INSERT(attrs, ATTRIBUTE_EARTH, "地");
    INSERT(attrs, ATTRIBUTE_WIND, "风");
    INSERT(attrs, ATTRIBUTE_DEVINE, "神");

    otherNamesDone = false;
    acc = false;
}

QString CardPool::getType(quint32 type)
{
    QList<QPair<int, QString> > ls;
    QStringList str;

    auto types = CardPool::getTypes();
    for(auto it = types.begin(); it != types.end(); ++it)
    {
        if(type & it.key())
        {
            ls.append(qMakePair(it.key(), it.value()));
        }
    }

    qSort(ls.begin(), ls.end(),
          [&](QPair<int, QString> &a, QPair<int, QString> &b)
    {
        return a.first < b.first;
    });

    foreach(auto &pair, ls)
    {
        str << pair.second;
    }

    return str.join('|');
}

QString CardPool::getRace(quint32 race)
{
    auto races = CardPool::getRaces();
    for(auto it = races.begin(); it != races.end(); ++it)
    {
        if(race & it.key())
        {
            return it.value();
        }
    }
    return QObject::tr("?");
}


static QString nameConv(QString name)
{
    QString conv;
    conv.reserve(name.length());
    for(int i : range(name.length()))
    {
        ushort unicode = name[i].unicode();
        if(unicode >= 65296 && unicode <= 65305)
        {
            conv.append(QChar(unicode - 65296));
        }
        else if(unicode >= 65313 && unicode <= 65338)
        {
            conv.append(QChar(unicode - 65313 + 'A'));
        }
        else if(unicode >= 65345 && unicode <= 65370)
        {
            conv.append(QChar(unicode - 65345 + 'a'));
        }
        else if(unicode == 8722)
        {
            conv.append(65293);
        }
        else if(unicode == ' ')
        {
            continue;
        }
        else
        {
            conv.append(name[i]);
        }
    }
    return conv;
}

void CardPool::Load(const QStringList &paths)
{
    thePool.reset(new CardPool());
    thePool->cdbPath = paths;
    thePool->newPool.reserve(10000);
    thePool->loadThread = QSharedPointer<LoadThread>::create(nullptr, thePool.data());
    foreach(auto &path, paths)
    {
        QSqlDatabase db;
        if(QSqlDatabase::contains("qt_sql_default_connection"))
        {
            db = QSqlDatabase::database("qt_sql_default_connection");
        }
        else
        {
            db = QSqlDatabase::addDatabase("QSQLITE");
        }

        db.setDatabaseName(path);
        db.open();

        QSqlQuery query;
        query.exec("select id from datas");
        if(query.first())
        {
            do
            {
                thePool->loadCard(query.value(0).toInt());
            } while(query.next());
        }

        db.close();
    }
}

void CardPool::loadOtherNames()
{
    if(thePool->otherNamesDone)
    {
        return;
    }
}

QString CardPool::getAttr(quint32 attribute)
{
    auto attrs = CardPool::getAttrs();
    for(auto it = attrs.begin(); it != attrs.end(); ++it)
    {
        if(attribute & it.key())
        {
            return it.value();
        }
    }
    return QObject::tr("?");
}


QSharedPointer<Card> CardPool::getNewCard(QString name, bool wait)
{
    name = nameConv(name);
    auto it = thePool->newPool.find(name);
    if(it == thePool->newPool.end() && thePool->loadThread && !thePool->otherNamesDone && wait)
    {
        thePool->acc = true;
        thePool->loadThread->wait();
        it = thePool->newPool.find(name);
    }
    if(it == thePool->newPool.end())
    {
        return QSharedPointer<Card>(nullptr);
    }
    return getCard(it.value());
}

void CardPool::LoadNames()
{
    thePool->loadThread->start();
}

QString Card::cardType()
{
    return std::move(CardPool::getType(type));
}

QString Card::cardRace()
{
    return std::move(CardPool::getRace(race));
}

QString Card::cardAttr()
{
    return std::move(CardPool::getAttr(attribute));
}

LoadThread::LoadThread(QObject *parent, CardPool *_thePool)
    : QThread(parent), ok(false), thePool(_thePool)
{

}

void LoadThread::run()
{
    sleep(3);
    foreach(auto &path, thePool->cdbPath)
    {
        QSqlDatabase db;
        if(QSqlDatabase::contains("qt_sql_default_connection"))
        {
            db = QSqlDatabase::database("qt_sql_default_connection");
        }
        else
        {
            db = QSqlDatabase::addDatabase("QSQLITE");
        }
        db.setDatabaseName(path);
        db.open();

        QSqlQuery query;
        query.exec("select id from datas");
        if(query.first())
        {
            do
            {
                if(!thePool->acc)
                {
                    msleep(10);
                }
                quint32 id = query.value(0).toUInt();
                QFile file("script/c" + QString::number(id) + ".lua");
                if(file.open(QFile::ReadOnly | QFile::Text))
                {
                    QString line = file.readLine();
                    line = line.trimmed();
                    if(line.length() <= 2)
                    {
                        continue;
                    }
                    QString name = line.mid(2);
                    thePool->newPool.insert(nameConv(name), id);
                }
                else
                {
                    QByteArray arr = expansions->open("script/c" + QString::number(id) + ".lua");
                    if(arr.isEmpty())
                    {
                        continue;
                    }
                    QString str(arr);
                    QTextStream in(&str);
                    QString line = in.readLine();
                    line = line.trimmed();
                    if(line.length() <= 2)
                    {
                        continue;
                    }
                    QString name = line.mid(2);
                    thePool->newPool.insert(nameConv(name), id);
                }
            }while(query.next());
        }
        db.close();
    }
    ok = true;
    thePool->otherNamesDone = true;
}
