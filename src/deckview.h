#ifndef DECKVIEW_H
#define DECKVIEW_H

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextStream>
#include <QSharedPointer>
#include <QEventLoop>
#include <QMessageBox>
#include <QFileInfo>
#include <QAction>
#include <QToolBar>
#include <QTabBar>

#include "decklabel.h"
#include "deckmodel.h"
#include "deckwidget.h"
#include "card.h"
#include "config.h"




class DeckView : public QWidget
{
    Q_OBJECT
public:
    friend class ItemThread;

    DeckView(QWidget *parent, QTabBar *_tabbar);
    DeckWidget *getMain()
    {
        return mainDeck;
    }

    DeckWidget *getExtra()
    {
        return extraDeck;
    }

    DeckWidget *getSide()
    {
        return sideDeck;
    }

    Type::DeckP getDeck()
    {
        auto deck = Type::DeckP::create();
        foreach(auto &item, mainDeck->getDeck())
        {
            deck->append(item.getId());
        }
        foreach(auto &item, extraDeck->getDeck())
        {
            deck->append(item.getId());
        }
        if(!sideHidden)
        {
            foreach(auto &item, sideDeck->getDeck())
            {
                deck->append(item.getId());
            }
        }
        return deck;
    }
signals:
    void currentIdChanged(int);
    void clickId(int);
    void details(int);
    void save(QString);
    void statusChanged(QString);
    void refreshLocals();
    void deckText(DeckModel*, bool, int);

public slots:

    void loadDeck(QString, QString, bool, bool);
    void loadRemoteDeck(QString, QString, bool);
    void saveDeck(QString);
    void saveSlot();

    void clearDeck();

    void setCurrentCardId(quint32 id)
    {
        mainDeck->setCurrentCardId(id);
        extraDeck->setCurrentCardId(id);
        sideDeck->setCurrentCardId(id);
    }

    void makeSnapShot(bool mod = true);
    void undo();
    void redo();
    void deleteDeck();

    void setStatus();
    void refresh();

    void abort()
    {
        getCurrentModel().abort();
        setReady(true);
    }

    void checkLeave()
    {
        mainDeck->checkLeave();
        extraDeck->checkLeave();
        sideDeck->checkLeave();
    }

    void newTab();
private slots:

    void setReady(bool t)
    {
        abortAction->setDisabled(t);
    }

    void help();
    void sort();
    void shuffle();
    void home();
    void print();
    void hideSide();
    void modelRefresh(int);
    void modelReady(int, bool);
    void closeTab(int);
private:
    DeckModel& addModel();
    DeckModel& getCurrentModel();
    int getTabIndexById(int id);
    void refreshTabs();
    void switchTab(int);

    typedef QSharedPointer<DeckModel> ModelP;
    QList<ModelP> models;
    ModelP currentModel;

    DeckWidget *mainDeck;
    DeckWidget *extraDeck;
    DeckWidget *sideDeck;
    DeckSizeLabel *st;
    QAction *undoAction, *redoAction;
    QAction *abortAction;
    QToolBar *toolbar;
    QTabBar *tabbar;
    bool sideHidden;
};

#endif // DECKVIEW_H
