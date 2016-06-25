#include "networking.h"
#include "configmanager.h"
#include <QTextDocument>

static std::unordered_map<size_t, std::weak_ptr<NetWorking>> mapping;
static int counter = 0;

Q_DECLARE_METATYPE(Type::DeckL)

NetWorking::NetWorking() : QObject(nullptr), m_timestamp(0), m_id(++counter)
{
}

void NetWorking::notify(int type, size_t timestamp, ptr res)
{
    if(timestamp != m_timestamp)
    {
        return;
    }

    switch(type)
    {
    case 0:
    {
        QString decki = engine->getString(res);
        emit deck(decki);
        break;
    }
    case 1:
    {
        auto ls = Type::DeckL::create();
        for(ptr i = res; Spairp(i); i = Scdr(i))
        {
            ptr t = Scar(i);
            QString name = engine->getString(Svector_ref(t, 1));
            QString id = engine->getString(Svector_ref(t, 0));
            QString type = engine->getString(Svector_ref(t, 2));
            QString tooltip = engine->getString(Svector_ref(t, 3));
            QTextDocument text;
            text.setHtml(name);
            name = text.toPlainText();
            text.setHtml(type);
            type = text.toPlainText();
            text.setHtml(tooltip);
            tooltip = text.toPlainText();
            QVariantList vls;
            vls << id;
            vls << type;
            vls << tooltip;
            ls->append(qMakePair(name, vls));
        }
        emit list(ls);
        break;
    }
    case 2:
    {
        QString cardName = engine->getString(res);
        emit name(cardName);
        break;
    }
    case 3:
    {
        Slock_object(res);
        emit packList(res);
        break;
    }
    case 4:
    {
        Slock_object(res);
        emit pack(res);
        break;
    }
    default:
        break;
    }
    emit ready(true);
    m_waiting = false;
}

void NetWorking::getDeck(QString id)
{
    with_scheme([&]()
    {
        engine->call("get-deck", Sfixnum(m_id), Sfixnum(++m_timestamp), engine->fromQString(id));
    });
    m_waiting = true;
    emit ready(false);
}

void NetWorking::getList(int page)
{
    with_scheme([&]()
    {
        engine->call("get-list", Sfixnum(m_id), Sfixnum(++m_timestamp),
                     Sfixnum(ConfigManager::inst().m_deckType), Sfixnum(ConfigManager::inst().m_Flt),
                     Sfixnum(page), engine->fromQString(ConfigManager::inst().m_tempConfig));
    });
    m_waiting = true;
    emit ready(false);
}

void NetWorking::getName(quint32 id)
{
    with_scheme([&]()
    {
        engine->call("get-name", Sfixnum(m_id), Sfixnum(++m_timestamp), Sunsigned(id));
    });
    m_waiting = true;
    emit ready(false);
}

void NetWorking::getPack(quint32 no)
{
    with_scheme([&]()
    {
        engine->call("get-pack", Sfixnum(m_id), Sfixnum(++m_timestamp), Sunsigned(no));
    });
    m_waiting = true;
    emit ready(false);
}

void NetWorking::getPackList()
{
    with_scheme([&]()
    {
        engine->call("get-pack-list", Sfixnum(m_id), Sfixnum(++m_timestamp));
    });
    m_waiting = true;
    emit ready(false);
}

void NetWorking::abort()
{
    ++m_timestamp;
    m_waiting = false;
    emit ready(true);
}

size_t NetWorking::getId() const
{
    return m_id;
}

bool NetWorking::getWaiting() const
{
    return m_waiting;
}

void notify(size_t id, int type, int timestamp, ptr data)
{
    auto it = mapping.find(id);
    if(it != mapping.end())
    {
        if(auto ptr = it->second.lock())
        {
            ptr->notify(type, timestamp, data);
        }
    }
}

std::shared_ptr<NetWorking> make_networking()
{

    auto t = qRegisterMetaType<Type::DeckL>();
    Q_UNUSED(t);
    auto ptr = std::make_shared<NetWorking>();
    mapping[ptr->getId()] = ptr;

    if(ptr->getId() % 100 == 0)
    {
        for(auto it = mapping.begin(); it != mapping.end();)
        {
            if(it->second.expired())
            {
                it = mapping.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }

    return ptr;
}
