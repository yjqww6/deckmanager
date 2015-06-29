#include "packview.h"
#include "iconbutton.h"

PackView::PackView(QWidget *parent, bool&moved)
    : QWidget(parent)
{
    tab = new QTabWidget(this);
    packList = new PackList;
    auto packEdit = new PackEditView(nullptr, moved);

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

    connect(packList, SIGNAL(editCards(QSharedPointer<QVector<int> >)),
            packEdit, SLOT(setCards(QSharedPointer<QVector<int> >)));
    connect(packList, SIGNAL(cards(QSharedPointer<QVector<int> >)),
            this, SIGNAL(cards(QSharedPointer<QVector<int> >)));
    connect(packList, SIGNAL(readPackOk()), this, SLOT(changeTab()));
    connect(packList, SIGNAL(packName(QString)), packEdit, SLOT(setName(QString)));
    connect(packEdit, SIGNAL(saved()), packList, SLOT(refresh()));
    connect(refreshButton, SIGNAL(clicked()), packList, SLOT(refresh()));
    connect(editButton, SIGNAL(clicked()), packList, SLOT(readPackEdit()));
    connect(packEdit, SIGNAL(details(int)), this, SIGNAL(details(int)));
    connect(this, SIGNAL(checkingLeave()), packEdit, SLOT(checkLeave()));
    tab->addTab(widget, config->getStr("tab", "list", "列表"));
    tab->addTab(packEdit, config->getStr("tab", "edit", "编辑"));
    vbox = new QVBoxLayout;
    vbox->addWidget(tab);
    setLayout(vbox);
}

