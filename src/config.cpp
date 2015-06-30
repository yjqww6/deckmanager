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
    limit = getStr("pref", "limit", "0").toInt();
    QFileInfo info(QFile(getStr("pref", "bg", "")));
    bg = info.exists();
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
    settings.setValue("pref/limit", limit);
}
