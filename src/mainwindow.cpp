#include "mainwindow.h"
#include "configmanager.h"
#include "iconbutton.h"
#include "card.h"
#include "carditem.h"
#include "cardslist.h"
#include "decklist.h"
#include "carddetails.h"
#include "locallist.h"
#include "cardfilter.h"
#include "replaylist.h"
#include "packview.h"
#include "pref.h"
#include "deckview.h"
#include "signaltower.h"
#include <QDebug>
#include <QSplitter>
#include <QTimer>
#include <QTabBar>
#include <functional>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_currentId(0)
{
    setWindowTitle("deckmanager - by qww6");
    if(ConfigManager::inst().m_bg)
    {
        QString bg = ConfigManager::inst().getStr("pref", "bg", "");
        setStyleSheet("QMainWindow{border-image: url(" + bg + ")}");
    }

    qint32 max = ConfigManager::inst().getStr("pref", "max", "0").toInt();
    qint32 width = ConfigManager::inst().getStr("pref", "width", "0").toInt();
    qint32 height = ConfigManager::inst().getStr("pref", "height", "0").toInt();

    if (width == 0)
    {
        width = size().width();
    }

    if (height == 0)
    {
        height = size().height();
    }

    resize(width, height);

    if (max > 0) {
        setWindowState(Qt::WindowMaximized);
    }

    auto modelTabBar = new QTabBar;
    auto sp = new QSplitter(Qt::Horizontal, this);
    sp->setHandleWidth(3);
    auto deckListView = new DeckListView;
    auto deckView = new DeckView(nullptr, modelTabBar);
    auto cardListView = new CardsListView(nullptr);

    auto cardDetails = new CardDetails;
    auto localList = new LocalList;
    auto filter = new CardFilter;

    auto replayList = new ReplayList;
    auto replayRefresh = new IconButton(":/icons/refresh.png", ConfigManager::inst().getStr("action", "refresh", "刷新"));

    auto packView = new PackView(nullptr);
    auto pref = new Pref;
    auto replayWidget = new QWidget;
    auto logEdit = new QPlainTextEdit;
    auto vbox = new QVBoxLayout;

    auto getCurrentResults = [=]() -> Type::Deck&
    {
        return cardListView->getList();
    };
    filter->m_getCurrent = getCurrentResults;

    auto getDeck = [=]() -> Type::DeckP
    {
        return deckView->getDeck();
    };
    filter->m_getDeck = getDeck;

    vbox->addWidget(replayList);
    vbox->addWidget(replayRefresh);
    replayWidget->setLayout(vbox);

    m_dialog = new ScriptView(this);

    m_tab = new MTabWidget;
    m_tab->setStyleSheet("QWidget{font-size: 13px}");
    cardDetails->setStyleSheet("QWidget{font-size: 15px}");

    connect(cardDetails, &CardDetails::clickId, m_dialog, &ScriptView::setId);
    connect(&SignalTower::inst(), &SignalTower::clickId, filter, &CardFilter::searchSet);
    connect(deckView, &DeckView::deckText, m_dialog, &ScriptView::setDeck);

    connect(replayRefresh, &IconButton::clicked, replayList, &ReplayList::refresh);

    connect(&SignalTower::inst(), &SignalTower::currentIdChanged, this, &MainWindow::changeId);

    connect(this, &MainWindow::currentIdChanged, [=](quint32 id) {
        cardDetails->setId(id);
        deckView->setCurrentCardId(id);
        cardListView->setCurrentCardId(id);
    });


    connect(deckListView, &DeckListView::selectDeck, deckView, &DeckView::loadRemoteDeck);
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

    connect(&SignalTower::inst(), &SignalTower::details, this, &MainWindow::toDetails);

    connect(deckView, &DeckView::save, [=](QString name){
        m_tab->setCurrentIndex(1, 0);
        localList->setPathFocus(name);
    });

    connect(deckView, &DeckView::statusChanged, this, &MainWindow::setWindowTitle);
    connect(deckView, &DeckView::refreshLocals, localList, &LocalList::refresh);

    auto timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [=]() {
       deckView->checkLeave();
       cardListView->checkLeave();
       packView->checkLeave();
    });
    timer->start(200);

    connect(&SignalTower::inst(), &SignalTower::debug, logEdit, &QPlainTextEdit::appendPlainText);
    logEdit->setPlainText(SignalTower::inst().m_accumulated);

    m_tab->addTabBar();
    m_tab->addTabBar();
    m_tab->addWidget(1, localList, ConfigManager::inst().getStr("tab", "local", "本地"));
    m_tab->addWidget(1, deckListView, ConfigManager::inst().getStr("tab", "remote", "远程"));
    m_tab->addWidget(1, replayWidget, ConfigManager::inst().getStr("tab", "replay", "录像"));
    m_tab->addWidget(1, logEdit, "Log");
    m_tab->addWidget(0, cardDetails, ConfigManager::inst().getStr("tab", "card", "卡"));
    m_tab->addWidget(0, filter, ConfigManager::inst().getStr("tab", "search", "卡池"));
    m_tab->addWidget(0, packView, ConfigManager::inst().getStr("tab", "pack", "卡包"));
    m_tab->addWidget(0, pref, ConfigManager::inst().getStr("tab", "pref", "选项"));
    m_tab->setCurrentIndex(1, 0);

    sp->addWidget(m_tab);
    sp->addWidget(deckView);
    sp->addWidget(cardListView);
    sp->setStretchFactor(1, 1);
    sp->setStyleSheet("QSplitter:handle{background:transparent}");

    m_tab->changeSize();

    auto hboxtop = new QHBoxLayout;

    auto vboxtop = new QVBoxLayout;
    hboxtop->addWidget(modelTabBar);


    auto toolbar = new QToolBar;

    auto newAction = new QAction(toolbar);
    newAction->setIcon(QIcon(":/icons/add.png"));
    newAction->setToolTip(ConfigManager::inst().getStr("action", "new", "新建"));

    toolbar->setStyleSheet("QToolTip{font-size:12px}QToolBar{background: rgba(255, 255, 255, 200)}");
    toolbar->setFixedHeight(24);
    toolbar->addAction(newAction);

    connect(newAction, &QAction::triggered, deckView, &DeckView::newTab);

    auto spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolbar->addWidget(spacer);

    hboxtop->addWidget(toolbar);

    vboxtop->addLayout(hboxtop);
    vboxtop->addWidget(sp, 1);
    auto widgettop = new QWidget;
    vboxtop->setMargin(0);
    vboxtop->setSpacing(0);
    hboxtop->setSpacing(0);

    widgettop->setLayout(vboxtop);
    setCentralWidget(widgettop);

    deckView->setStatus();
    deckListView->getList(1);
    localList->refresh();
    cardListView->refresh();
    replayList->refresh();
    packView->refresh();
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Q && event->modifiers() == Qt::AltModifier)
    {
        ConfigManager::inst().m_autoSwitch = !ConfigManager::inst().m_autoSwitch;
        if(ConfigManager::inst().m_autoSwitch)
        {
            m_tab->setCurrentIndex(0, 0);
        }
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::toDetails(quint32)
{
    static QPair<int, int> prev = m_tab->currentIndex();

    if(m_tab->currentIndex() == qMakePair(0, 0))
    {
        m_tab->setCurrentIndex(prev);
    }
    else
    {
        prev = m_tab->currentIndex();
        m_tab->setCurrentIndex(0, 0);
    }
}

void MainWindow::changeId(quint32 id)
{
    if(id == m_currentId)
    {
        return;
    }

    m_currentId = id;

    if(!(QApplication::keyboardModifiers() & Qt::ShiftModifier))
    {
        emit currentIdChanged(id);
        if(ConfigManager::inst().m_autoSwitch)
        {
            m_tab->setCurrentIndex(0, 0);
        }
    }
}

MainWindow::~MainWindow()
{

}
