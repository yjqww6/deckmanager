#include "config.h"
#include <QFileInfo>
#include <QDebug>

Config *config = nullptr;
RemoteConfig tempRemoteConfig("Temp");

Config::Config(QObject *parent)
    : QObject(parent), remote(0)
{
    QSettings settings("dmconfig.ini", QSettings::IniFormat);
    settings.setIniCodec("utf8");
    QStringList groups = settings.value("remote/groups", QStringList()).toStringList();
    foreach(auto group, settings.childGroups())
    {
        settings.beginGroup(group);
        foreach(auto key, settings.allKeys())
        {
            mappings.insert(group + "/" + key, settings.value(key));
        }
        settings.endGroup();
    }
    waitForPass = getStr("pref", "passwait", "0").toInt() == 1;
    convertPass = getStr("pref", "passconvert", "1").toInt() == 1;
    autoSwitch = getStr("pref", "autoswitch", "0").toInt() == 1;
    newTab = getStr("pref", "newTab", "0").toInt() == 1;
    limit = getStr("pref", "limit", "0").toInt();
    QFileInfo info(QFile(getStr("pref", "bg", "")));
    bg = info.exists();

    foreach(auto group, groups)
    {
        QString parent = getStr(group, "parent", "");
        if(parent.isEmpty())
        {
            remoteConfigs.append(RemoteConfig(mappings, group, nullptr));
        }
        else
        {
            RemoteConfig rc(parent);
            auto it = qFind(remoteConfigs.begin(), remoteConfigs.end(), rc);
            if(it != remoteConfigs.end())
            {
                remoteConfigs.append(RemoteConfig(mappings, group, &(*it)));
            }
        }
    }
}

RemoteConfig::RemoteConfig(QString group)
{
    id = group;
    str = group;
}

RemoteConfig& RemoteConfig::operator =(const RemoteConfig &other)
{
    id = other.id;
    str = other.str;
    codec = other.codec;
    getlist = other.getlist;
    getlistparam = other.getlistparam;
    finishlist = other.finishlist;
    deckname = other.deckname;
    deckid = other.deckid;
    decktype = other.decktype;
    decktooltip = other.decktooltip;
    getdeck = other.getdeck;
    finishdeck = other.finishdeck;
    deck = other.deck;
    getname = other.getname;
    finishname = other.finishname;
    name = other.name;
    openurl = other.openurl;
    return *this;
}

RemoteConfig::RemoteConfig(Map &mappings, QString group, RemoteConfig *parent)
{
    auto finder = [&](QString key) -> QString {
        QString ckey = group + "/" + key;
        auto it = mappings.find(ckey);
        if(it != mappings.end())
        {
            return it.value().toString();
        }
        else
        {
            return QString("");
        }
    };

#define FILL(field) \
    do {\
        QString found = finder(#field);\
        if(found.isNull() || found.isEmpty())\
        {\
            if(parent)\
            {\
                field = parent->field;\
            }\
        }\
        else\
        {\
            field = found;\
        }\
    } while(false)

    id = group;
    FILL(str);
    FILL(codec);
    FILL(getlist);
    FILL(getlistparam);
    FILL(finishlist);
    FILL(deckname);
    FILL(deckid);
    FILL(decktype);
    FILL(decktooltip);
    FILL(getdeck);
    FILL(finishdeck);
    FILL(deck);
    FILL(getname);
    FILL(finishname);
    FILL(name);
    FILL(openurl);
}

RemoteConfig bad("bad");

RemoteConfig& Config::getCurrentRemote()
{
    if(remote >= 0 && remote < remoteConfigs.size())
    {
        return remoteConfigs[remote];
    }
    else if(remote == -1)
    {
        return tempRemoteConfig;
    }
    else
    {
        return bad;
    }
}

QString Config::getStr(QString group, QString key, QString defaultStr)
{
    auto it = mappings.find(group + "/" + key);
    if(it != mappings.end())
    {
        return it.value().toString();
    }
    else
    {
//        return QString();
        return defaultStr;
    }
}

QString Config::getHelpStr()
{
    auto it = mappings.find("help/help");
    if(it != mappings.end())
    {
        return it.value().toStringList().join('\n');
    }
    return QString();
}

void Config::setWaitForPass(bool t)
{
    waitForPass = t;
}

void Config::setConvertPass(bool t)
{
    convertPass = t;
}


void Config::setLimit(int i)
{
    limit = i;
}

void Config::setRemote(int i)
{
    remote = i;
}

void Config::setAutoSwitch(bool t)
{
    autoSwitch = t;
}

Config::~Config()
{
    QSettings settings("dmconfig.ini", QSettings::IniFormat);
    settings.setIniCodec("utf8");
    settings.setValue("pref/passwait", waitForPass ? 1 : 0);
    settings.setValue("pref/passconvert", convertPass ? 1 : 0);
    settings.setValue("pref/autoswitch", autoSwitch ? 1 : 0);
    settings.setValue("pref/newTab", newTab ? 1 : 0);
    settings.setValue("pref/limit", limit);
}
void Config::setNewTab(bool value)
{
    newTab = value;
}

