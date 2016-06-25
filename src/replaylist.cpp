#include "replaylist.h"
#include "configmanager.h"

ReplayList::ReplayList(QWidget *parent)
    : QTreeWidget(parent)
{
    setColumnCount(2);
    setColumnHidden(1, true);
    setHeaderHidden(true);
    connect(this, &ReplayList::itemClicked, this, &ReplayList::readYrp);

    m_popup = new QMenu(this);
    auto newTabAction = new QAction(m_popup);
    newTabAction->setText(ConfigManager::inst().getStr("action", "newtab", "在新标签页打开"));
    m_popup->addAction(newTabAction);

    connect(newTabAction, &QAction::triggered, [=]()
    {
        if(m_menuItem && m_menuItem->parent())
        {
            sendDeck(m_menuItem, true);
        }
    });
}

void ReplayList::contextMenuEvent(QContextMenuEvent *)
{
    m_menuItem = itemAt(mapFromGlobal(QCursor::pos()));
    if(m_menuItem && m_menuItem->parent())
    {
        m_popup->exec(QCursor::pos());
    }
}

void ReplayList::refresh()
{
    QDir qdir("replay/");
    QStringList filter;
    filter << "*.yrp";

    clear();
    m_decks.clear();

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
                it->setData(0, Qt::UserRole, m_decks.size());
                m_decks.append(std::move(player.second));
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
    auto &deck = m_decks[index];
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

