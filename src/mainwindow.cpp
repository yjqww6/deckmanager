#include "mainwindow.h"
#include "config.h"
#include "iconbutton.h"
#include "card.h"
#include "carditem.h"
#include "cardslist.h"
#include "decklist.h"
#include "carddetails.h"
#include "remote.h"
#include "locallist.h"
#include "cardfilter.h"
#include "replaylist.h"
#include "packview.h"
#include "pref.h"
#include "deckview.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSplitter>
#include <QTimer>
#include <functional>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentId(0)
{
    setWindowTitle("deckmanager - by qww6");
    if(config->bg)
    {
        QString bg = config->getStr("pref", "bg", "");
        setStyleSheet("QMainWindow{border-image: url(" + bg + ")}");
    }

    auto sp = new QSplitter(Qt::Horizontal, this);
    sp->setHandleWidth(5);
    auto deckListView = new DeckListView;
    auto deckView = new DeckView(nullptr);
    auto cardListView = new CardsListView(nullptr);
    if(config->bg)
    {
        cardListView->setStyleSheet("color: white; font-size: 15px");
    }
    else
    {
        cardListView->setStyleSheet("font-size: 15px");
    }

    auto cardDetails = new CardDetails;
    auto localList = new LocalList;
    auto filter = new CardFilter;

    auto replayList = new ReplayList;
    auto replayRefresh = new IconButton(":/icons/refresh.png", config->getStr("action", "refresh", "刷新"));

    auto packView = new PackView(nullptr);
    auto pref = new Pref;
    auto widget = new QWidget;
    auto vbox = new QVBoxLayout;
    auto getter = [=]() -> QVector<int>& {
        return cardListView->getList();
    };
    filter->getCurrent = getter;
    vbox->addWidget(replayList);
    vbox->addWidget(replayRefresh);
    widget->setLayout(vbox);

    dialog = new ScriptView(this);

    tab = new MTabWidget;
    tab->setStyleSheet("font-size: 13px;");

    tab->addTabBar();
    tab->addTabBar();
    tab->addWidget(1, localList, config->getStr("tab", "local", "本地"));
    tab->addWidget(1, deckListView, config->getStr("tab", "remote", "远程"));
    tab->addWidget(1, widget, config->getStr("tab", "replay", "录像"));
    tab->addWidget(0, cardDetails, config->getStr("tab", "card", "卡"));
    tab->addWidget(0, filter, config->getStr("tab", "search", "卡池"));
    tab->addWidget(0, packView, config->getStr("tab", "pack", "卡包"));
    tab->addWidget(0, pref, config->getStr("tab", "pref", "选项"));
    tab->setCurrentIndex(1, 0);

    cardDetails->setStyleSheet("font-size: 15px");

    sp->addWidget(tab);
    sp->addWidget(deckView);
    sp->addWidget(cardListView);
    sp->setStretchFactor(1, 1);
    sp->setStyleSheet("QSplitter:handle{background:transparent}");

    connect(cardDetails, &CardDetails::clickId, dialog, &ScriptView::setId);
    connect(deckView, &DeckView::clickId, dialog, &ScriptView::setId);
    connect(cardListView, &CardsListView::clickId, dialog, &ScriptView::setId);

    connect(replayRefresh, &IconButton::clicked, replayList, &ReplayList::refresh);

    connect(deckView, &DeckView::currentIdChanged, this, &MainWindow::changeId);
    connect(cardListView, &CardsListView::currentIdChanged, this, &MainWindow::changeId);

    connect(this, &MainWindow::currentIdChanged, cardDetails, &CardDetails::setId);
    connect(this, &MainWindow::currentIdChanged, deckView, &DeckView::setCurrentCardId);
    connect(this, &MainWindow::currentIdChanged, cardListView, &CardsListView::setCurrentCardId);

    setCentralWidget(sp);

    connect(deckListView, &DeckListView::deckStream, deckView, &DeckView::loadDeck);
    connect(localList, &LocalList::deckStream, deckView, &DeckView::loadDeck);
    connect(localList, &LocalList::saveDeck, deckView, &DeckView::saveDeck);
    connect(filter, &CardFilter::result, cardListView, &CardsListView::setCards);

    connect(replayList, &ReplayList::deckStream,deckView, &DeckView::loadDeck);
    connect(packView, &PackView::cards, cardListView, &CardsListView::setCards);

    connect(pref, &Pref::lflistChanged, [=]() {
        deckView->update();
        cardListView->update();
        packView->update();
    });
    connect(pref, &Pref::lfList, cardListView, &CardsListView::setCards);

    connect(cardListView, &CardsListView::details, this, &MainWindow::toDetails);
    connect(packView, &PackView::details, this, &MainWindow::toDetails);
    connect(deckView, &DeckView::details, this, &MainWindow::toDetails);

    connect(deckView, &DeckView::save, this, &MainWindow::save);
    connect(deckView, &DeckView::save, localList, &LocalList::setPathFocus);

    connect(deckView, &DeckView::statusChanged, this, &MainWindow::setWindowTitle);
    connect(deckView, &DeckView::refreshLocals, localList, &LocalList::refresh);

    connect(this, &MainWindow::destroyed, CardPool::getThread(), &LoadThread::terminate);

    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, deckView, &DeckView::checkLeave);
    connect(timer, &QTimer::timeout, cardListView, &CardsListView::checkLeave);
    connect(timer, &QTimer::timeout, packView, &PackView::checkingLeave);
    timer->start(200);

    deckView->setStatus();
    deckListView->getList();
    localList->refresh();
    cardListView->refresh();
    replayList->refresh();
    packView->refresh();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Q && event->modifiers() == Qt::AltModifier)
    {
        config->autoSwitch = !config->autoSwitch;
        if(config->autoSwitch)
        {
            tab->setCurrentIndex(0, 0);
        }
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::toDetails(int)
{
    static QPair<int, int> prev = tab->currentIndex();

    if(tab->currentIndex() == qMakePair(0, 0))
    {
        tab->setCurrentIndex(prev);
    }
    else
    {
        prev = tab->currentIndex();
        tab->setCurrentIndex(0, 0);
    }
}

void MainWindow::save()
{
    tab->setCurrentIndex(1, 0);
}

void MainWindow::changeId(int id)
{
    if(id == currentId)
    {
        return;
    }

    currentId = id;

    emit currentIdChanged(id);
    if(config->autoSwitch)
    {
        tab->setCurrentIndex(0, 0);
    }
}

MainWindow::~MainWindow()
{

}
