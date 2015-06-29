#include "locallist.h"
#include "iconbutton.h"
#include "config.h"
#include <QDebug>

LocalList::LocalList(QWidget *parent)
    : QWidget(parent)
{
    listWidget = new QListWidget;
    auto vbox = new QVBoxLayout;
    auto hbox = new QHBoxLayout;

    pathEdit = new QLineEdit;
    buttonSave = new IconButton(":/icons/saveas.png", config->getStr("action", "saveas", "另存为"));
    buttonRefresh = new IconButton(":/icons/refresh.png", config->getStr("action", "refresh", "刷新"));


    vbox->addWidget(listWidget);
    hbox->addWidget(pathEdit);
    hbox->addWidget(buttonSave);
    vbox->addLayout(hbox);
    vbox->addWidget(buttonRefresh);
    setLayout(vbox);

    connect(buttonRefresh, SIGNAL(clicked()), this, SLOT(refresh()));
    connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(deckStreamTrans(QListWidgetItem*)));
    connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(itemName(QListWidgetItem*)));
    connect(buttonSave, SIGNAL(clicked()), this, SLOT(saveDeckTrans()));
}

void LocalList::refresh()
{
    QDir qdir("deck/");
    QStringList filter;
    filter << "*.ydk";
    listWidget->clear();
    foreach(QFileInfo info, qdir.entryInfoList(filter))
    {
        auto item = new QListWidgetItem;
        item->setText(info.completeBaseName());
        item->setData(Qt::UserRole, info.filePath());
        listWidget->addItem(item);
    }
}

void LocalList::itemName(QListWidgetItem *item)
{
    QString path = item->text();
    pathEdit->setText(path);
}

void LocalList::deckStreamTrans(QListWidgetItem *item)
{
    QString path = item->data(Qt::UserRole).toString();
    QFile file(path);
    if(file.open(QFile::ReadOnly))
    {
        QString str = file.readAll();
        emit deckStream(str, QFileInfo(file).completeBaseName(), true);
        file.close();
    }
}


void LocalList::saveDeckTrans()
{
    emit saveDeck("deck/" + pathEdit->text() + ".ydk");
    refresh();
}

LocalList::~LocalList()
{

}

