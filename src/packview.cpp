#include "packview.h"
#include "iconbutton.h"

PackView::PackView(QWidget *parent)
    : QWidget(parent)
{
    m_tab = new QTabWidget(this);
    m_packList = new PackList;
    m_remotePackList = new RemotePackList;
    auto packEdit = new PackEditView(nullptr);

    auto widget = new QWidget;
    auto vbox = new QVBoxLayout;
    auto refreshButton = new IconButton(":/icons/refresh.png", ConfigManager::inst().getStr("action", "refresh", "刷新"));
    auto editButton = new IconButton(":/icons/edit.png", ConfigManager::inst().getStr("action", "edit", "编辑"));
    vbox->addWidget(m_packList, 1);

    auto hbox = new QHBoxLayout;
    hbox->addWidget(editButton);
    hbox->addWidget(refreshButton);
    vbox->addLayout(hbox);
    widget->setLayout(vbox);

    auto widget2 = new QWidget;
    auto vbox2 = new QVBoxLayout;
    auto refreshButton2 = new IconButton(":/icons/refresh.png", ConfigManager::inst().getStr("action", "refresh", "刷新"));
    auto abortButton = new IconButton(":/icons/abort.png", ConfigManager::inst().getStr("action", "abort", "中止"));
    auto editButton2 = new IconButton(":/icons/edit.png", ConfigManager::inst().getStr("action", "edit", "编辑"));
    vbox2->addWidget(m_remotePackList, 1);
    auto hbox2 = new QHBoxLayout;
    hbox2->addWidget(editButton2);
    hbox2->addWidget(refreshButton2);
    hbox2->addWidget(abortButton);
    vbox2->addLayout(hbox2);
    widget2->setLayout(vbox2);
    abortButton->setDisabled(true);

    connect(m_packList, &PackList::editCards, packEdit, &PackEditView::setCards);
    connect(m_packList, &PackList::cards, this, &PackView::cards);
    connect(m_packList, &PackList::readPackOk, this, &PackView::changeTab);
    connect(m_packList, &PackList::packName, packEdit, &PackEditView::setName);

    connect(m_remotePackList, &RemotePackList::editCards, packEdit, &PackEditView::setCards);
    connect(m_remotePackList, &RemotePackList::cards, this, &PackView::cards);
    connect(m_remotePackList, &RemotePackList::readPackOk, this, &PackView::changeTab);
    connect(m_remotePackList, &RemotePackList::packName, packEdit, &PackEditView::setName);
    connect(m_remotePackList->m_net.get(), &NetWorking::ready, abortButton, &IconButton::setDisabled);
    connect(m_remotePackList->m_net.get(), &NetWorking::ready, refreshButton2, &IconButton::setEnabled);

    connect(packEdit, &PackEditView::saved, m_packList, &PackList::refresh);
    connect(refreshButton, &IconButton::clicked, m_packList, &PackList::refresh);
    connect(editButton, &IconButton::clicked, m_packList, &PackList::readPackEdit);

    connect(refreshButton2, &IconButton::clicked, m_remotePackList, &RemotePackList::refresh);
    connect(abortButton, &IconButton::clicked, m_remotePackList->m_net.get(), &NetWorking::abort);
    connect(editButton2, &IconButton::clicked, m_remotePackList, &RemotePackList::readPackEdit);

    m_tab->addTab(widget, ConfigManager::inst().getStr("tab", "list", "列表"));
    m_tab->addTab(widget2, ConfigManager::inst().getStr("tab", "remotep", "在线"));
    m_tab->addTab(packEdit, ConfigManager::inst().getStr("tab", "edit", "编辑"));
    vbox = new QVBoxLayout;
    vbox->addWidget(m_tab);
    setLayout(vbox);
}

void PackView::refresh()
{
    m_packList->refresh();
}

void PackView::changeTab()
{
    m_tab->setCurrentIndex(2);
}

