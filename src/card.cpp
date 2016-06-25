#include "card.h"
#include "expansions.h"
#include "configmanager.h"
#include "range.h"
#include "engine.h"
#include <QDebug>
#include "sqlite3/sqlite3.h"

optional<Card*> CardManager::getCard(quint32 id)
{
    auto it = m_cards.find(id);
    if(it == m_cards.end())
    {
        return nullopt;
    }
    return it->second.get();
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

QString CardManager::getType(quint32 type)
{
    return with_scheme([type]()
    {

        ptr str = engine->call("type-string", Sfixnum(type));
        return engine->getString(str);
    });
}

QString CardManager::getRace(quint32 race)
{
    for(auto it = m_races.begin(); it != m_races.end(); ++it)
    {
        if(race & it.key())
        {
            return it.value();
        }
    }
    return "?";
}

QString CardManager::getAttr(quint32 attribute)
{
    for(auto it = m_attrs.begin(); it != m_attrs.end(); ++it)
    {
        if(attribute & it.key())
        {
            return it.value();
        }
    }
    return "?";
}


optional<Card*> CardManager::getNewCard(QString name, bool wait)
{
    return with_scheme([&]()
    {
        ptr str = engine->call("utf8->string", engine->bytevector(name.toUtf8()));
        ptr id = engine->call("orig->id", str, Sboolean(wait));
        return getCard(Sfixnum_value(id));
    });
}



void CardManager::loadCardData(QStringList paths)
{

#define INSERT(container, field, str) \
    do { \
        auto &cont = container;\
        auto name = ConfigManager::inst().getStr("string", #field, str);\
        cont.insert(Const::field, name);\
    } while(false)

    INSERT(m_types, TYPE_MONSTER, "怪兽");
    INSERT(m_types, TYPE_SPELL, "魔法");
    INSERT(m_types, TYPE_TRAP, "陷阱");
    INSERT(m_types, TYPE_NORMAL, "通常");
    INSERT(m_types, TYPE_EFFECT, "效果");
    INSERT(m_types, TYPE_FUSION, "融合");
    INSERT(m_types, TYPE_RITUAL, "仪式");
    INSERT(m_types, TYPE_TRAPMONSTER, "陷阱怪兽");
    INSERT(m_types, TYPE_SPIRIT, "灵魂");
    INSERT(m_types, TYPE_UNION, "同盟");
    INSERT(m_types, TYPE_DUAL, "二重");
    INSERT(m_types, TYPE_TUNER, "调整");
    INSERT(m_types, TYPE_SYNCHRO, "同调");
    INSERT(m_types, TYPE_TOKEN, "衍生物");
    INSERT(m_types, TYPE_QUICKPLAY, "速攻");
    INSERT(m_types, TYPE_CONTINUOUS, "永续");
    INSERT(m_types, TYPE_EQUIP, "装备");
    INSERT(m_types, TYPE_FIELD, "场地");
    INSERT(m_types, TYPE_COUNTER, "反击");
    INSERT(m_types, TYPE_FLIP, "反转");
    INSERT(m_types, TYPE_TOON, "卡通");
    INSERT(m_types, TYPE_XYZ, "XYZ");
    INSERT(m_types, TYPE_PENDULUM, "灵摆");
    INSERT(m_races, RACE_WARRIOR, "战士");
    INSERT(m_races, RACE_SPELLCASTER, "魔法师");
    INSERT(m_races, RACE_FAIRY, "天使");
    INSERT(m_races, RACE_FIEND, "恶魔");
    INSERT(m_races, RACE_ZOMBIE, "不死");
    INSERT(m_races, RACE_MACHINE, "机械");
    INSERT(m_races, RACE_AQUA, "水");
    INSERT(m_races, RACE_PYRO, "炎");
    INSERT(m_races, RACE_ROCK, "岩石");
    INSERT(m_races, RACE_WINDBEAST, "鸟兽");
    INSERT(m_races, RACE_PLANT, "植物");
    INSERT(m_races, RACE_INSECT, "昆虫");
    INSERT(m_races, RACE_THUNDER, "雷");
    INSERT(m_races, RACE_DRAGON, "龙");
    INSERT(m_races, RACE_BEAST, "兽");
    INSERT(m_races, RACE_BEASTWARRIOR, "兽战士");
    INSERT(m_races, RACE_DINOSAUR, "恐龙");
    INSERT(m_races, RACE_FISH, "鱼");
    INSERT(m_races, RACE_SEASERPENT, "海龙");
    INSERT(m_races, RACE_REPTILE, "爬虫");
    INSERT(m_races, RACE_PSYCHO, "念动力");
    INSERT(m_races, RACE_DEVINE, "幻神兽");
    INSERT(m_races, RACE_CREATORGOD, "创造神");
    INSERT(m_races, RACE_PHANTOMDRAGON, "幻龙");


    INSERT(m_attrs, ATTRIBUTE_DARK, "暗");
    INSERT(m_attrs, ATTRIBUTE_LIGHT, "光");
    INSERT(m_attrs, ATTRIBUTE_WATER, "水");
    INSERT(m_attrs, ATTRIBUTE_FIRE, "火");
    INSERT(m_attrs, ATTRIBUTE_EARTH, "地");
    INSERT(m_attrs, ATTRIBUTE_WIND, "风");
    INSERT(m_attrs, ATTRIBUTE_DEVINE, "神");

    m_newPool.reserve(10000);
    foreach(auto &path, paths)
    {
        loadDataBase(path, m_cards);
    }
}
