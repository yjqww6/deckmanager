#include "configmanager.h"
#include "engine.h"
#include <QFileInfo>
#include <QDebug>

QString ConfigManager::getStr(QString group, QString key, QString defaultStr)
{
    auto it = m_configs.find(group + "/" + key);
    if(it != m_configs.end())
    {
        return it.value().toString();
    }
    else
    {
        return defaultStr;
    }
}

QString ConfigManager::getHelpStr()
{
    auto it = m_configs.find("help/help");
    if(it != m_configs.end())
    {
        return it.value().toStringList().join('\n');
    }
    return QString();
}

void ConfigManager::loadConfig()
{
    QSettings settings("dmconfig.ini", QSettings::IniFormat);
    settings.setIniCodec("utf8");

    foreach(auto group, settings.childGroups())
    {
        settings.beginGroup(group);
        foreach(auto key, settings.allKeys())
        {
            m_configs.insert(group + "/" + key, settings.value(key));
        }
        settings.endGroup();
    }
    m_waitForPass = getStr("pref", "passwait", "0").toInt() == 1;
    m_convertPass = getStr("pref", "passconvert", "1").toInt() == 1;
    m_autoSwitch = getStr("pref", "autoswitch", "0").toInt() == 1;
    m_newTab = getStr("pref", "newTab", "0").toInt() == 1;
    m_limit = getStr("pref", "limit", "0").toInt();
    m_usesetname = getStr("pref", "usesetname", "1").toInt() == 1;

    with_scheme([&]()
    {
       Sset_top_level_value(Sstring_to_symbol("use-setname?"), Sboolean(m_usesetname));
    });

    QFileInfo info(QFile(getStr("pref", "bg", "")));
    m_bg = info.exists();
    m_deckType = 0;
    m_Flt = 0;

    settings.beginGroup("Flt");
    foreach(QString key, settings.childKeys())
    {
        m_Flts.append(qMakePair(key.toInt(), settings.value(key).toString()));
    }

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
                m_deckTypes.append(qMakePair(deckType, name));
            }
        }
        std::reverse(m_deckTypes.begin(), m_deckTypes.end());
    }
}

void ConfigManager::saveConfig()
{
    QSettings settings("dmconfig.ini", QSettings::IniFormat);
    settings.setIniCodec("utf8");
    settings.setValue("pref/passwait", m_waitForPass ? 1 : 0);
    settings.setValue("pref/passconvert", m_convertPass ? 1 : 0);
    settings.setValue("pref/autoswitch", m_autoSwitch ? 1 : 0);
    settings.setValue("pref/newTab", m_newTab ? 1 : 0);
    settings.setValue("pref/limit", m_limit);
    settings.setValue("pref/usesetname", m_usesetname ? 1 : 0);
}

void ConfigManager::setWaitForPass(bool t)
{
    m_waitForPass = t;
}

void ConfigManager::setConvertPass(bool t)
{
    m_convertPass = t;
}


void ConfigManager::setLimit(int i)
{
    m_limit = i;
}

void ConfigManager::setAutoSwitch(bool t)
{
    m_autoSwitch = t;
}

ConfigManager::ConfigManager(QObject *parent) : QObject(parent)
{

}

ConfigManager::~ConfigManager()
{
}

void ConfigManager::setNewTab(bool value)
{
    m_newTab = value;
}

void ConfigManager::setUseSetName(bool val)
{
    m_usesetname = val;
    with_scheme([&]()
    {
       Sset_top_level_value(Sstring_to_symbol("use-setname?"), Sboolean(val));
    });
}

