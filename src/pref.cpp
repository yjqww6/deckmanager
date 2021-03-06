#include "pref.h"
#include "limitcards.h"
#include "config.h"
#include "iconbutton.h"
#include <QCheckBox>

Pref::Pref(QWidget *parent) : QWidget(parent)
{
    auto lf = new QLabel(config->getStr("label", "limit", "禁卡表"));
    auto getButton = new IconButton(":/icons/right.png");
    lfcombo = new QComboBox();
    auto tables = limitCards->getTables();
    auto it = tables.begin();
    int index = 0;
    for(; it != tables.end(); ++it, ++index)
    {
        lfcombo->addItem(it->first, index);
    }
    lfcombo->addItem(config->getStr("label", "noupperbound", "无上限"), -1);

    auto waitC = new QCheckBox(config->getStr("label", "passwait", "卡密缺失等待"));
    auto convertC = new QCheckBox(config->getStr("label", "passconvert", "先行/正式卡密转换"));
    auto newTabC = new QCheckBox(config->getStr("label", "newtab", "总是在新标签打开"));
    auto setnameC = new QCheckBox(config->getStr("label", "usesetname", "使用字段名"));

    waitC->setChecked(config->waitForPass);
    convertC->setChecked(config->convertPass);
    lfcombo->setCurrentIndex(lfcombo->count() >= config->limit ? config->limit : 0);
    config->setLimit(lfcombo->currentData().toInt());
    newTabC->setChecked(config->newTab);
    setnameC->setChecked(config->usesetname);

    connect(lfcombo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &Pref::setLflist);
    connect(getButton, &IconButton::clicked, this, &Pref::openLfList);
    connect(waitC, &QCheckBox::toggled, config, &Config::setWaitForPass);
    connect(convertC, &QCheckBox::toggled, config, &Config::setConvertPass);
    connect(newTabC, &QCheckBox::toggled, config, &Config::setNewTab);
    connect(setnameC, &QCheckBox::toggled, config, &Config::setUseSetName);

    auto vbox = new QVBoxLayout;
    auto lfbox = new QHBoxLayout;


    lfbox->addWidget(lf);
    lfbox->addWidget(lfcombo);
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
    config->setLimit(lfcombo->itemData(index).toInt());
    emit lflistChanged();
}

void Pref::openLfList()
{
    emit lfList(limitCards->getCards(lfcombo->currentIndex()));
}
