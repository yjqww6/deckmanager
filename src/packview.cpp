#include "packview.h"
#include "iconbutton.h"

PackView::PackView(QWidget *parent)
    : QWidget(parent)
{
    tab = new QTabWidget(this);
    packList = new PackList;
    auto packEdit = new PackEditView(nullptr);

    auto widget = new QWidget;
    auto vbox = new QVBoxLayout;
    auto refreshButton = new IconButton(":/icons/refresh.png", config->getStr("action", "refresh", "刷新"));
    auto editButton = new IconButton(":/icons/edit.png", config->getStr("action", "edit", "编辑"));
    vbox->addWidget(packList, 1);

    auto hbox = new QHBoxLayout;
    hbox->addWidget(editButton);
    hbox->addWidget(refreshButton);
    vbox->addLayout(hbox);
    widget->setLayout(vbox);

    connect(packList, &PackList::editCards, packEdit, &PackEditView::setCards);
    connect(packList, &PackList::cards, this, &PackView::cards);
    connect(packList, &PackList::readPackOk, this, &PackView::changeTab);
    connect(packList, &PackList::packName, packEdit, &PackEditView::setName);
    connect(packEdit, &PackEditView::saved, packList, &PackList::refresh);
    connect(refreshButton, &IconButton::clicked, packList, &PackList::refresh);
    connect(editButton, &IconButton::clicked, packList, &PackList::readPackEdit);
    connect(packEdit, &PackEditView::details, this, &PackView::details);
    connect(this, &PackView::checkLeave, packEdit, &PackEditView::checkLeave);
    tab->addTab(widget, config->getStr("tab", "list", "列表"));
    tab->addTab(packEdit, config->getStr("tab", "edit", "编辑"));
    vbox = new QVBoxLayout;
    vbox->addWidget(tab);
    setLayout(vbox);
}

