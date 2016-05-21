#include "config.h"
#include <QFileInfo>
#include <QDebug>

Config *config = nullptr;

Config::Config(QObject *parent)
    : QObject(parent)
{
    QSettings settings("dmconfig.ini", QSettings::IniFormat);
    settings.setIniCodec("utf8");

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
    usesetname = getStr("pref", "usesetname", "1").toInt() == 1;
    QFileInfo info(QFile(getStr("pref", "bg", "")));
    bg = info.exists();
    deckType = 0;
    Flt = 0;

    settings.beginGroup("Flt");
    foreach(QString key, settings.childKeys())
    {
        Flts.append(qMakePair(key.toInt(), settings.value(key).toString()));
    }

    remoteConfig.set(mappings, "remote");

    QFile file("decktype.ini");
    if(file.open(QFile::ReadOnly))
    {
        QTextStream in(&file);
        for(QString line = in.readLine(); !line.isNull(); line = in.readLine())
        {
            int i = line.indexOf('=');
            if(i >= 0 && line.length() > (i + 1))
            {
                int deckType = line.left(i).toInt();
                QString name = line.mid(i + 1);
                deckTypes.append(qMakePair(deckType, name));
            }
        }
        std::reverse(deckTypes.begin(), deckTypes.end());
    }
}

RemoteConfig::RemoteConfig()
{

}

void RemoteConfig::set(Map &mappings, QString group)
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
        field = finder(#field);\
    } while(false)

    FILL(codec);
    FILL(getlist);
    FILL(getlistparam);
    FILL(getlistparamwithdecktype);
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

RemoteConfig& Config::getCurrentRemote()
{
    return remoteConfig;
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
    settings.setValue("pref/usesetname", usesetname ? 1 : 0);
}
void Config::setNewTab(bool value)
{
    newTab = value;
}

void Config::setUseSetName(bool val)
{
    usesetname = val;
}

