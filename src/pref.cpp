#include "pref.h"
#include "limitcards.h"
#include "configmanager.h"
#include "iconbutton.h"
#include "engine.h"
#include <QCheckBox>

Pref::Pref(QWidget *parent) : QWidget(parent)
{
    auto lf = new QLabel(ConfigManager::inst().getStr("label", "limit", "禁卡表"));
    auto getButton = new IconButton(":/icons/right.png");
    m_lfcombo = new QComboBox();
    auto tables = LimitCards::inst().m_tables;
    auto it = tables.begin();
    int index = 0;
    for(; it != tables.end(); ++it, ++index)
    {
        m_lfcombo->addItem(it->first, index);
    }
    m_lfcombo->addItem(ConfigManager::inst().getStr("label", "noupperbound", "无上限"), -1);

    auto waitC = new QCheckBox(ConfigManager::inst().getStr("label", "passwait", "卡密缺失等待"));
    auto convertC = new QCheckBox(ConfigManager::inst().getStr("label", "passconvert", "先行/正式卡密转换"));
    auto newTabC = new QCheckBox(ConfigManager::inst().getStr("label", "newtab", "总是在新标签打开"));
    auto setnameC = new QCheckBox(ConfigManager::inst().getStr("label", "usesetname", "使用字段名"));

    waitC->setChecked(ConfigManager::inst().m_waitForPass);
    convertC->setChecked(ConfigManager::inst().m_convertPass);
    m_lfcombo->setCurrentIndex(m_lfcombo->count() >= ConfigManager::inst().m_limit ? ConfigManager::inst().m_limit : 0);
    ConfigManager::inst().m_limit = m_lfcombo->currentData().toInt();
    newTabC->setChecked(ConfigManager::inst().m_newTab);
    setnameC->setChecked(ConfigManager::inst().m_usesetname);

    connect(m_lfcombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &Pref::setLflist);
    connect(getButton, &IconButton::clicked, this, &Pref::openLfList);
    connect(waitC, &QCheckBox::toggled, [](bool b)
    {
        ConfigManager::inst().m_waitForPass = b;
    });
    connect(convertC, &QCheckBox::toggled, [](bool b)
    {
        ConfigManager::inst().m_convertPass = b;
    });
    connect(newTabC, &QCheckBox::toggled, [](bool b)
    {
        ConfigManager::inst().m_newTab = b;
    });
    connect(setnameC, &QCheckBox::toggled, [](bool b)
    {
        ConfigManager::inst().m_usesetname = b;
        with_scheme([&]()
        {
           Sset_top_level_value(Sstring_to_symbol("use-setname?"), Sboolean(b));
        });
    });

    auto vbox = new QVBoxLayout;
    auto lfbox = new QHBoxLayout;


    lfbox->addWidget(lf);
    lfbox->addWidget(m_lfcombo);
    lfbox->addWidget(getButton);
    vbox->addLayout(lfbox);
    vbox->addWidget(waitC);
    vbox->addWidget(convertC);
    vbox->addWidget(newTabC);
    vbox->addWidget(setnameC);
    setLayout(vbox);
}


void Pref::setLflist(int index)
{
    ConfigManager::inst().m_limit = m_lfcombo->itemData(index).toInt();
    emit lflistChanged();
}

void Pref::openLfList()
{
    emit lfList(LimitCards::inst().getCards(m_lfcombo->currentIndex()));
}
