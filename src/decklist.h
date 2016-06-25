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
#include <QSpinBox>
#include <QDesktopServices>
#include <QInputDialog>
#include <QCheckBox>
#include <QMenu>
#include "networking.h"

class DeckList : public QListWidget
{
    Q_OBJECT
public:
    DeckList(QWidget *parent = 0);
    ~DeckList();
    void contextMenuEvent(QContextMenuEvent *);
signals:
    void selectDeck(QString, QString, bool);
    void deckType(QString);
public slots:
    void setList(Type::DeckL);
private slots:
    void onItemChanged();
    void onItem(QListWidgetItem *);
    void openURL();
    void newTab();
    void same();
private:
    QMenu *m_popup;
    QListWidgetItem *m_menuItem;
};

class DeckListView : public QWidget
{
    Q_OBJECT
public:
    DeckListView(QWidget *parent = 0);

signals:
    void selectDeck(QString, QString, bool);
public slots:
    void setList(Type::DeckL);
    void getList(int);

private slots:
    void goPage(int);
private:
    DeckList *m_decklist;
    std::shared_ptr<NetWorking> m_networking;
    QSpinBox *m_pageBox;
    int m_lastConfig;
    int m_lastPage;
};

#endif // DECKLIST_H
