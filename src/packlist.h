#ifndef PACKLIST_H
#define PACKLIST_H

#include <QWidget>
#include <QTreeWidget>
#include <QDir>
#include "card.h"
#include "types.h"
#include "networking.h"

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
    virtual void refresh();
    void readPackEdit()
    {
        m_edited = true;
        readPack(currentItem(), currentColumn());
        emit readPackOk();
    }

private slots:
    virtual void readPack(QTreeWidgetItem *, int);
    void readPackClk(QTreeWidgetItem *item, int i)
    {
        m_edited = false;
        readPack(item, i);
    }

protected:
    QTreeWidgetItem *m_myPack;
    bool            m_edited;
    QString         m_encoding;
};

class RemotePackList : public PackList
{
    Q_OBJECT
public:
    explicit RemotePackList(QWidget *parent = 0);
public slots:
    virtual void refresh();
private slots:
    virtual void readPack(QTreeWidgetItem *, int);

    void pack(ptr ls);
    void packList(ptr ls);
public:
    std::shared_ptr<NetWorking> m_net;
};

#endif // PACKLIST_H
