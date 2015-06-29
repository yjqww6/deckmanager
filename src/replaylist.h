#ifndef REPLAYLIST_H
#define REPLAYLIST_H

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QList>
#include <QFileInfo>
#include <QDir>
#include <QSharedPointer>
#include <QTextStream>
#include "yrp.h"

class ReplayList : public QTreeWidget
{
    Q_OBJECT
public:
    ReplayList(QWidget *parent = 0);
    ~ReplayList();
signals:
    void deckStream(QString, QString, bool);
public slots:
    void refresh();
private slots:
    void readYrp(QTreeWidgetItem*, int);

private:
    QList<QVector<int> > decks;
};

#endif // REPLAYLIST_H
