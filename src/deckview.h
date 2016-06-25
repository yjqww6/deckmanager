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
#include "configmanager.h"




class DeckView : public QWidget
{
    Q_OBJECT
public:
    friend class ItemThread;

    DeckView(QWidget *parent, QTabBar *_tabbar);
    DeckWidget *getMain()
    {
        return m_mainDeck;
    }

    DeckWidget *getExtra()
    {
        return m_extraDeck;
    }

    DeckWidget *getSide()
    {
        return m_sideDeck;
    }

    Type::DeckP getDeck()
    {
        auto deck = Type::DeckP::create();
        foreach(auto &item, m_mainDeck->getDeck())
        {
            deck->append(item.getId());
        }
        foreach(auto &item, m_extraDeck->getDeck())
        {
            deck->append(item.getId());
        }
        if(!m_sideHidden)
        {
            foreach(auto &item, m_sideDeck->getDeck())
            {
                deck->append(item.getId());
            }
        }
        return deck;
    }
signals:
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
        m_mainDeck->setCurrentCardId(id);
        m_extraDeck->setCurrentCardId(id);
        m_sideDeck->setCurrentCardId(id);
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
        m_mainDeck->checkLeave();
        m_extraDeck->checkLeave();
        m_sideDeck->checkLeave();
    }

    void newTab();
private slots:

    void setReady(bool t)
    {
        m_abortAction->setDisabled(t);
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
    QList<ModelP> m_models;
    ModelP m_currentModel;

    DeckWidget *m_mainDeck;
    DeckWidget *m_extraDeck;
    DeckWidget *m_sideDeck;
    DeckSizeLabel *st;
    QAction *m_undoAction, *m_redoAction;
    QAction *m_abortAction;
    QToolBar *m_toolbar;
    QTabBar *m_tabbar;
    bool m_sideHidden;
};

#endif // DECKVIEW_H
