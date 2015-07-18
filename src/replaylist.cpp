#include "replaylist.h"
#include "config.h"

ReplayList::ReplayList(QWidget *parent)
    : QTreeWidget(parent)
{
    setColumnCount(2);
    setColumnHidden(1, true);
    setHeaderHidden(true);
    connect(this, &ReplayList::itemClicked, this, &ReplayList::readYrp);

    popup = new QMenu(this);
    auto newTabAction = new QAction(popup);
    newTabAction->setText(config->getStr("action", "newtab", "在新标签页打开"));
    popup->addAction(newTabAction);

    connect(newTabAction, &QAction::triggered, [=]()
    {
        if(menuItem && menuItem->parent())
        {
            sendDeck(menuItem, true);
        }
    });
}

void ReplayList::contextMenuEvent(QContextMenuEvent *)
{
    menuItem = itemAt(mapFromGlobal(QCursor::pos()));
    if(menuItem && menuItem->parent())
    {
        popup->exec(QCursor::pos());
    }
}

void ReplayList::refresh()
{
    QDir qdir("replay/");
    QStringList filter;
    filter << "*.yrp";

    clear();
    decks.clear();

    QList<QTreeWidgetItem*> items;

    foreach(QFileInfo info, qdir.entryInfoList(filter))
    {
        auto item = new QTreeWidgetItem;
        item->setText(0, info.baseName());
        item->setData(0, Qt::UserRole, info.filePath());
        item->setData(1, Qt::UserRole, false);
        items.append(item);
    }

    insertTopLevelItems(0, items);
}

void ReplayList::readYrp(QTreeWidgetItem *item, int)
{
    if(item->parent() == nullptr)
    {
        if(!item->data(1, Qt::UserRole).toBool())
        {
            Yrp yrp(item->data(0, Qt::UserRole).toString());

            foreach(const Yrp::Item &player, yrp.decks)
            {
                auto it = new QTreeWidgetItem(item, QStringList(player.first));
                it->setData(0, Qt::UserRole, decks.size());
                decks.append(std::move(player.second));
                item->addChild(it);
            }
            item->setData(1, Qt::UserRole, true);
        }
    }
    else
    {
        sendDeck(item, false);
    }
}

void ReplayList::sendDeck(QTreeWidgetItem *item, bool newTab)
{
    if(!item)
    {
        return;
    }
    int index = item->data(0, Qt::UserRole).toInt();
    auto &deck = decks[index];
    QStringList ls;
    foreach(quint32 id, deck)
    {
        ls << QString::number(id);
    }
    QString text = ls.join('\n');
    emit deckStream(text, item->parent()->text(0) + "-" + item->text(0), false, newTab);
}

ReplayList::~ReplayList()
{

}

