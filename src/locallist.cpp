#include "locallist.h"
#include "iconbutton.h"
#include "config.h"
#include <QMessageBox>
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

    popup = new QMenu(this);
    auto newTabAction = new QAction(popup);
    newTabAction->setText(config->getStr("action", "newtab", "在新标签页打开"));
    auto deleteAction = new QAction(popup);
    deleteAction->setText(config->getStr("action", "delete", "删除卡组"));
    popup->addAction(newTabAction);
    popup->addAction(deleteAction);

    vbox->addWidget(listWidget);
    hbox->addWidget(pathEdit);
    hbox->addWidget(buttonSave);
    vbox->addLayout(hbox);
    vbox->addWidget(buttonRefresh);
    setLayout(vbox);

    connect(buttonRefresh, &IconButton::clicked, this, &LocalList::refresh);
    connect(listWidget, &QListWidget::itemClicked, [=](QListWidgetItem *item) {
        sendDeck(item, false);
    });
    connect(listWidget, &QListWidget::itemClicked, this, &LocalList::itemName);
    connect(buttonSave, &IconButton::clicked, this, &LocalList::saveDeckTrans);
    connect(newTabAction, &QAction::triggered, [=]() {
        if(menuItem)
        {
            sendDeck(menuItem, true);
        }
    });
    connect(deleteAction, &QAction::triggered, this, &LocalList::deleteDeck);
}

void LocalList::contextMenuEvent(QContextMenuEvent *)
{
    menuItem = listWidget->itemAt(listWidget->mapFromGlobal(QCursor::pos()));
    if(menuItem)
    {
        popup->exec(QCursor::pos());
    }
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

void LocalList::sendDeck(QListWidgetItem *item, bool newTab)
{
    QString path = item->data(Qt::UserRole).toString();
    QFile file(path);
    if(file.open(QFile::ReadOnly))
    {
        QString str = file.readAll();
        emit deckStream(str, QFileInfo(file).completeBaseName(), true, newTab);
        file.close();
    }
}

void LocalList::deleteDeck()
{
    if(menuItem)
    {
        QString path = menuItem->data(Qt::UserRole).toString();
        QFile file(path);
        if(QMessageBox::question(nullptr, config->getStr("label", "warning", "警告"),
                                 config->getStr("label", "delete_p", "是否要删除卡组:")
                                 + QFileInfo(file).completeBaseName() + "?",
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
                == QMessageBox::Yes)
        {
            if(file.remove())
            {
                refresh();
            }
        }
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

