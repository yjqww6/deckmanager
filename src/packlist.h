#ifndef PACKLIST_H
#define PACKLIST_H

#include <QWidget>
#include <QTreeWidget>
#include <QDir>
#include "card.h"
#include "typing.h"

class PackList : public QTreeWidget
{
    Q_OBJECT
public:
    explicit PackList(QWidget *parent = 0);
signals:
    void cards(Type::DeckP);
    void editCards(Type::DeckP);
    void readPackOk();
    void packName(QString);
public slots:
    void refresh();
    void readPackEdit()
    {
        edited = true;
        readPack(currentItem(), currentColumn());
        emit readPackOk();
    }

private slots:
    void readPack(QTreeWidgetItem *, int);
    void readPackClk(QTreeWidgetItem *item, int i)
    {
        edited = false;
        readPack(item, i);
    }

private:
    QTreeWidgetItem *myPack;
    bool edited;
    QString encoding;
};

#endif // PACKLIST_H
