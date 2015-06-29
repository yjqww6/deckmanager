#include "replaylist.h"

ReplayList::ReplayList(QWidget *parent)
    : QTreeWidget(parent)
{
    setColumnCount(2);
    setColumnHidden(1, true);
    setHeaderHidden(true);
    connect(this, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(readYrp(QTreeWidgetItem*,int)));
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
        int index = item->data(0, Qt::UserRole).toInt();
        auto &deck = decks[index];
        QStringList ls;
        foreach(int id, deck)
        {
            ls << QString::number(id);
        }
        QString text = ls.join('\n');
        emit deckStream(text, item->parent()->text(0) + "-" + item->text(0), false);
    }
}

ReplayList::~ReplayList()
{

}

