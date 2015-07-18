#ifndef REPLAYLIST_H
#define REPLAYLIST_H

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QList>
#include <QFileInfo>
#include <QDir>
#include <QSharedPointer>
#include <QTextStream>
#include <QMenu>
#include "yrp.h"
#include "typing.h"

class ReplayList : public QTreeWidget
{
    Q_OBJECT
public:
    ReplayList(QWidget *parent = 0);
    ~ReplayList();
    void contextMenuEvent(QContextMenuEvent *);
signals:
    void deckStream(QString, QString, bool, bool);
public slots:
    void refresh();
    void sendDeck(QTreeWidgetItem*, bool);
private slots:
    void readYrp(QTreeWidgetItem*, int);

private:

    QList<Type::Deck> decks;

    QMenu *popup;
    QTreeWidgetItem *menuItem;
};

#endif // REPLAYLIST_H
