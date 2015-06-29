#ifndef DECKLIST_H
#define DECKLIST_H

#include <QListWidget>
#include <QListWidgetItem>
#include <QList>
#include <QString>
#include <QPair>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QDesktopServices>
#include <QInputDialog>
#include <QCheckBox>
#include "remote.h"

class DeckList : public QListWidget
{
    Q_OBJECT
public:
    DeckList(QWidget *parent = 0);
    ~DeckList();

signals:
    void selectDeck(QString, QString);
public slots:
    void setList(QSharedPointer<QList<QPair<QString, QString> > >);
private slots:
    void onItemChanged();
    void onItem(QListWidgetItem *);
    void openURL(QListWidgetItem*);
};

class DeckListView : public QWidget
{
    Q_OBJECT
public:
    DeckListView(QWidget *parent = 0);

signals:
    void deckStream(QString, QString, bool);

public slots:
    void setList(QSharedPointer<QList<QPair<QString, QString> > >);
    void getList()
    {
        remote.getList(page);
    }

private slots:
    void nextPage();
    void prevPage();
    void goPage();
private:
    DeckList *decklist;
    Remote remote;
    int page;
    QCheckBox *allCards;
    QLineEdit *pageEdit;
};

#endif // DECKLIST_H
