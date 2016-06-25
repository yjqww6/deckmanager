#include "locallist.h"
#include "iconbutton.h"
#include "configmanager.h"
#include <QMessageBox>
#include <QDebug>

LocalList::LocalList(QWidget *parent)
    : QWidget(parent)
{
    m_listWidget = new QListWidget;
    auto vbox = new QVBoxLayout;
    auto hbox = new QHBoxLayout;

    m_pathEdit = new QLineEdit;
    m_buttonSave = new IconButton(":/icons/saveas.png", ConfigManager::inst().getStr("action", "saveas", "另存为"));
    m_buttonRefresh = new IconButton(":/icons/refresh.png", ConfigManager::inst().getStr("action", "refresh", "刷新"));
    m_includeAI = new QCheckBox("AI");
    m_includeAI->setChecked(false);

    m_popup = new QMenu(this);
    auto newTabAction = new QAction(m_popup);
    newTabAction->setText(ConfigManager::inst().getStr("action", "newtab", "在新标签页打开"));
    auto deleteAction = new QAction(m_popup);
    deleteAction->setText(ConfigManager::inst().getStr("action", "delete", "删除卡组"));
    m_popup->addAction(newTabAction);
    m_popup->addAction(deleteAction);

    vbox->addWidget(m_listWidget);
    hbox->addWidget(m_pathEdit);
    hbox->addWidget(m_buttonSave);
    vbox->addLayout(hbox);

    hbox = new QHBoxLayout;
    hbox->addWidget(m_includeAI);
    hbox->addWidget(m_buttonRefresh, 1);
    vbox->addLayout(hbox);
    setLayout(vbox);

    connect(m_buttonRefresh, &IconButton::clicked, this, &LocalList::refresh);
    connect(m_listWidget, &QListWidget::itemClicked, [=](QListWidgetItem *item) {
        sendDeck(item, false);
    });
    connect(m_listWidget, &QListWidget::itemClicked, this, &LocalList::itemName);
    connect(m_buttonSave, &IconButton::clicked, this, &LocalList::saveDeckTrans);
    connect(newTabAction, &QAction::triggered, [=]() {
        if(m_menuItem)
        {
            sendDeck(m_menuItem, true);
        }
    });
    connect(deleteAction, &QAction::triggered, this, &LocalList::deleteDeck);
    connect(m_includeAI, &QCheckBox::toggled, [=](bool) {
        refresh();
    });
}

void LocalList::contextMenuEvent(QContextMenuEvent *)
{
    m_menuItem = m_listWidget->itemAt(m_listWidget->mapFromGlobal(QCursor::pos()));
    if(m_menuItem)
    {
        m_popup->exec(QCursor::pos());
    }
}


void LocalList::refresh()
{
    QDir qdir("deck/");
    QStringList filter;
    filter << "*.ydk";
    m_listWidget->clear();
    foreach(QFileInfo info, qdir.entryInfoList(filter))
    {
        QString name = info.completeBaseName();
        if(!m_includeAI->isChecked() && name.startsWith("AI"))
        {
            continue;
        }
        auto item = new QListWidgetItem;
        item->setText(name);
        item->setData(Qt::UserRole, info.filePath());
        m_listWidget->addItem(item);
    }
}

void LocalList::itemName(QListWidgetItem *item)
{
    QString path = item->text();
    m_pathEdit->setText(path);
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
    if(m_menuItem)
    {
        QString path = m_menuItem->data(Qt::UserRole).toString();
        QFile file(path);
        if(QMessageBox::question(nullptr, ConfigManager::inst().getStr("label", "warning", "警告"),
                                 ConfigManager::inst().getStr("label", "delete_p", "是否要删除卡组:")
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
    emit saveDeck("deck/" + m_pathEdit->text() + ".ydk");
    refresh();
}

LocalList::~LocalList()
{

}

