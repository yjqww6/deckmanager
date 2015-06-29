#include "mainwindow.h"
#include "config.h"
#include "iconbutton.h"
#include <QTimer>
#include <functional>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentId(0)
{
    setWindowTitle("deckmanager - by qww6");
    auto sp = new QSplitter(Qt::Horizontal, this);
    sp->setHandleWidth(5);
    auto deckListView = new DeckListView;
    auto deckView = new DeckView(nullptr, movedGlobal);
    auto cardListView = new CardsListView(nullptr, movedGlobal);
    auto cardDetails = new CardDetails;
    auto localList = new LocalList;
    auto filter = new CardFilter;

    auto replayList = new ReplayList;
    auto replayRefresh = new IconButton(":/icons/refresh.png", config->getStr("action", "refresh", "刷新"));

    auto packView = new PackView(nullptr, movedGlobal);
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

    tab = new QTabWidget;

    setFont(QFont(font().family(), 11));


    tab->addTab(localList, config->getStr("tab", "local", "本地"));
    tab->addTab(deckListView, config->getStr("tab", "remote", "远程"));
    tab->addTab(widget, config->getStr("tab", "replay", "录像"));
    tab->addTab(cardDetails, config->getStr("tab", "card", "卡"));
    tab->addTab(filter, config->getStr("tab", "search", "卡池"));
    tab->addTab(packView, config->getStr("tab", "pack", "卡包"));
    tab->addTab(pref, config->getStr("tab", "pref", "选项"));
    tab->setMinimumWidth(250);


    sp->addWidget(tab);
    sp->addWidget(deckView);
    sp->addWidget(cardListView);

    connect(cardDetails, SIGNAL(clickId(int)), dialog, SLOT(setId(int)));
    connect(deckView, SIGNAL(clickId(int)), dialog, SLOT(setId(int)));
    connect(cardListView, SIGNAL(clickId(int)), dialog, SLOT(setId(int)));

    connect(replayRefresh, SIGNAL(clicked()), replayList, SLOT(refresh()));

    connect(deckView, SIGNAL(currentIdChanged(int)), this, SLOT(changeId(int)));
    connect(cardListView, SIGNAL(currentIdChanged(int)), this, SLOT(changeId(int)));

    connect(this, SIGNAL(currentIdChanged(int)), cardDetails, SLOT(setId(int)));
    connect(this, SIGNAL(currentIdChanged(int)), deckView, SLOT(setCurrentCardId(int)));
    connect(this, SIGNAL(currentIdChanged(int)), cardListView, SLOT(setCurrentCardId(int)));

    setCentralWidget(sp);

    connect(deckListView, SIGNAL(deckStream(QString, QString, bool)),
                     deckView, SLOT(loadDeck(QString, QString, bool)));

    connect(localList, SIGNAL(deckStream(QString, QString, bool)),
                     deckView, SLOT(loadDeck(QString, QString, bool)));

    connect(localList, SIGNAL(saveDeck(QString)), deckView, SLOT(saveDeck(QString)));

    connect(filter, SIGNAL(result(QSharedPointer<QVector<int> >)),
            cardListView, SLOT(setCards(QSharedPointer<QVector<int> >)));

    connect(replayList, SIGNAL(deckStream(QString, QString, bool)),
                               deckView, SLOT(loadDeck(QString, QString, bool)));
    connect(packView, SIGNAL(cards(QSharedPointer<QVector<int> >)),
            cardListView, SLOT(setCards(QSharedPointer<QVector<int> >)));

    connect(pref, SIGNAL(lflistChanged()), deckView, SLOT(update()));
    connect(pref, SIGNAL(lflistChanged()), cardListView, SLOT(update()));
    connect(pref, SIGNAL(lflistChanged()), packView, SLOT(update()));
    connect(pref, SIGNAL(lfList(QSharedPointer<QVector<int> >)),
            cardListView, SLOT(setCards(QSharedPointer<QVector<int> >)));

    connect(cardListView, SIGNAL(details(int)), this, SLOT(toDetails(int)));
    connect(packView, SIGNAL(details(int)), this, SLOT(toDetails(int)));
    connect(deckView, SIGNAL(details(int)), this, SLOT(toDetails(int)));

    connect(deckView, SIGNAL(save()), this, SLOT(save()));
    connect(deckView, SIGNAL(save()), localList, SLOT(setPathFocus()));

    connect(deckView, SIGNAL(statusChanged(QString)), this, SLOT(setWindowTitle(QString)));
    connect(deckView, SIGNAL(refreshLocals()), localList, SLOT(refresh()));

    connect(this, SIGNAL(destroyed()), CardPool::getThread(), SLOT(terminate()));

    auto timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), deckView, SLOT(checkLeave()));
    connect(timer, SIGNAL(timeout()), cardListView, SLOT(checkLeave()));
    connect(timer, SIGNAL(timeout()), packView, SIGNAL(checkingLeave()));
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
            tab->setCurrentIndex(3);
        }
    }

    QMainWindow::keyPressEvent(event);
}

void MainWindow::toDetails(int)
{
    static int prev = tab->currentIndex();
    static bool switched = false;

    if(switched)
    {
        tab->setCurrentIndex(prev);
        switched = false;
    }
    else
    {
        prev = tab->currentIndex();
        tab->setCurrentIndex(3);
        switched = true;
    }
}

void MainWindow::save()
{
    tab->setCurrentIndex(0);
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
        tab->setCurrentIndex(3);
    }
}

MainWindow::~MainWindow()
{

}
