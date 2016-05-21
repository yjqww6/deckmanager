#include "card.h"
#include "expansions.h"
#include "config.h"
#include "range.h"
#include <QDebug>
#include "sqlite3/sqlite3.h"

CardPool *cardPool = nullptr;

Wrapper<Card> CardPool::getCard(quint32 id)
{
    auto it = pool.find(id);
    if(it == pool.end())
    {
        return Wrapper<Card>();
    }
    return Wrapper<Card>(it->second.get());
}


static bool sqlError(sqlite3 *db, sqlite3_stmt *stmt = nullptr)
{
    if (stmt)
    {
        sqlite3_finalize(stmt);
    }
    sqlite3_close(db);

    return false;
}

static bool loadDataBase(const QString &fileName, std::unordered_map<quint32, std::unique_ptr<Card>> &pool)
{
    sqlite3 *DB = nullptr;
    if (sqlite3_open(fileName.toLatin1().data(), &DB) != SQLITE_OK)
    {
        return sqlError(DB);
    }

    sqlite3_stmt* stmt = nullptr;
    const char *sql = "select * from datas,texts where datas.id=texts.id";

    if (sqlite3_prepare_v2(DB, sql, -1, &stmt, 0) != SQLITE_OK)
    {
        return sqlError(DB, stmt);
    }

    int step = 0;
    do {
        step = sqlite3_step(stmt);
        if (step == SQLITE_BUSY || step == SQLITE_ERROR || step == SQLITE_MISUSE)
            return sqlError(DB, stmt);
        else if (step == SQLITE_ROW) {
            uint32_t code = sqlite3_column_int(stmt, 0);
            Card &cd = *pool.emplace(std::piecewise_construct,
                std::forward_as_tuple(code), std::forward_as_tuple(new Card)).first->second;

            cd.id = code;
            cd.ot = sqlite3_column_int(stmt, 1);
            cd.alias = sqlite3_column_int(stmt, 2);
            cd.setcode = sqlite3_column_int64(stmt, 3);
            cd.type = sqlite3_column_int(stmt, 4);
            cd.atk = sqlite3_column_int(stmt, 5);
            cd.def = sqlite3_column_int(stmt, 6);
            unsigned int level = sqlite3_column_int(stmt, 7);
            cd.level = level & 0xff;
            cd.scale = (level >> 24) & 0xff;
            cd.race = sqlite3_column_int(stmt, 8);
            cd.attribute = sqlite3_column_int(stmt, 9);
            cd.category = sqlite3_column_int(stmt, 10);
            cd.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12));
            cd.effect = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13));
        }
    } while (step != SQLITE_DONE);
    sqlite3_finalize(stmt);
    sqlite3_close(DB);
    return true;
}

CardPool::CardPool(QStringList paths) : loadThread(nullptr, this)
{
#define INSERT(container, field, str) \
    do { \
        auto &cont = container;\
        auto name = config->getStr("string", #field, str);\
        cont.insert(Const::field, name);\
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

    loadSetNames();

    cdbPath = paths;
    newPool.reserve(10000);
    foreach(auto &path, cdbPath)
    {
        loadDataBase(path, pool);
    }
}

QString CardPool::getType(quint32 type)
{
    QList<QPair<int, QString> > ls;
    QStringList str;

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
    for(auto it = races.begin(); it != races.end(); ++it)
    {
        if(race & it.key())
        {
            return it.value();
        }
    }
    return "?";
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

QString CardPool::getAttr(quint32 attribute)
{
    for(auto it = attrs.begin(); it != attrs.end(); ++it)
    {
        if(attribute & it.key())
        {
            return it.value();
        }
    }
    return "?";
}


Wrapper<Card> CardPool::getNewCard(QString name, bool wait)
{
    name = nameConv(name);
    QMutexLocker locker(&mutex);
    auto it = newPool.find(name);
    if(it == newPool.end() && !otherNamesDone && wait)
    {
        acc = true;
        locker.unlock();
        loadThread.wait();
        locker.relock();
        it = newPool.find(name);
    }
    if(it == newPool.end())
    {
        return Wrapper<Card>();
    }
    return Wrapper<Card>(getCard(it.value()));
}

void CardPool::loadNames()
{
    loadThread.start();
}

void CardPool::loadSetNames()
{
    QFile file("setcode.conf");
    if(file.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream stream(&file);
        stream.setCodec(QTextCodec::codecForName("gbk"));
        QString line;
        while(stream.readLineInto(&line))
        {
            if(!line.startsWith("0x"))
            {
                continue;
            }
            int it = line.indexOf(' ', 2);
            if(it == -1)
            {
                continue;
            }
            bool ok = false;
            quint32 setcode = line.mid(2, it - 2).toULong(&ok, 16);
            if(ok)
            {
                QString setname = line.mid(it + 1);
                setnames[setcode] = setname;
                setnamesR[setname] = setcode;
            }
        }
        return;
    }

    QFile conf("strings.conf");
    if(conf.open(QFile::ReadOnly | QFile::Text))
    {
        QTextStream stream(&conf);
        stream.setCodec(QTextCodec::codecForName("utf8"));
        QString line;
        while(stream.readLineInto(&line))
        {
            if(!line.startsWith("!setname 0x"))
            {
                continue;
            }
            int it = line.indexOf(' ', 11);
            if(it == -1)
            {
                continue;
            }
            bool ok = false;
            quint32 setcode = line.mid(11, it - 11).toULong(&ok, 16);
            if(ok)
            {
                int it2 = line.indexOf('\t', it + 1);
                QString setname = line.mid(it + 1, it2 - it - 1);
                setnames[setcode] = setname;
                setnamesR[setname] = setcode;
            }
        }
    }
}

QString Card::cardType()
{
    return cardPool->getType(type);
}

QString Card::cardRace()
{
    return cardPool->getRace(race);
}

QString Card::cardAttr()
{
    return cardPool->getAttr(attribute);
}

LoadThread::LoadThread(QObject *parent, CardPool *_thePool)
    : QThread(parent), thePool(_thePool)
{

}

void LoadThread::run()
{
    sleep(3);
    QList<quint32> ls;
    for(auto &it : cardPool->pool)
    {
        ls.append(it.first);
    }
    qSort(ls);
    for(quint32 id : ls)
    {
        if(!thePool->acc)
        {
            msleep(10);
        }

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

            QMutexLocker locker(&thePool->mutex);
            thePool->newPool.insert(nameConv(name), id);
        }
        else
        {
            QByteArray arr = expansions->open("script/c" + QString::number(id) + ".lua");
            if(arr.isEmpty())
            {
                continue;
            }
            QTextStream in(&arr);
            QString line = in.readLine();
            line = line.trimmed();
            if(line.length() <= 2)
            {
                continue;
            }
            QString name = line.mid(2);
            QMutexLocker locker(&thePool->mutex);
            thePool->newPool.insert(nameConv(name), id);
        }
    }
    thePool->otherNamesDone = true;
}
