#include "deckview.h"
#include "limitcards.h"
#include "config.h"
#include <QDebug>
#include <QDesktopServices>
#include <QFileDialog>
#include <QResizeEvent>
#include <QUrl>
#include <QScreen>
#include <QToolButton>


class ExtraDeckWidget : public DeckWidget
{
public:
    ExtraDeckWidget(QWidget *parent)
        : DeckWidget(parent, 1, 10)
    {

    }

    virtual bool filter(int id)
    {
        auto card = CardPool::getCard(id);
        return card && card->inExtra();
    }
};

class MainDeckWidget : public DeckWidget
{
public:
    MainDeckWidget(QWidget *parent)
        : DeckWidget(parent, 4, 10)
    {

    }
    virtual bool filter(int id)
    {
        auto card = CardPool::getCard(id);
        return card && !card->inExtra();
    }
};

void DeckView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
}

DeckView::DeckView(QWidget *parent)
    : QWidget(parent), currentLoad(0), waiting(false), sideHidden(false)
{
    if(config->bg)
    {
        setStyleSheet("color: white; font-size: 16px");
    }
    else
    {
        setStyleSheet("font-size: 16px");
    }
    auto vbox = new QVBoxLayout;
    auto hbox = new QHBoxLayout;
    mainDeck = new MainDeckWidget(nullptr);

    auto t = new DeckSizeLabel(config->getStr("label", "main", "主卡组"));
    auto mt = new MainDeckLabel;


    toolbar = new QToolBar;
    toolbar->setStyleSheet("color: black; font-size: 12px");

    undoAction = new QAction(toolbar);
    undoAction->setIcon(QIcon(":/icons/undo.png"));
    undoAction->setToolTip(config->getStr("action", "undo", "撤销"));
    toolbar->addAction(undoAction);

    redoAction = new QAction(toolbar);
    redoAction->setIcon(QIcon(":/icons/redo.png"));
    redoAction->setToolTip(config->getStr("action", "redo", "重做"));
    toolbar->addAction(redoAction);

    toolbar->addSeparator();

    auto newAction = new QAction(toolbar);
    newAction->setIcon(QIcon(":/icons/new.png"));
    newAction->setToolTip(config->getStr("action", "new", "新建"));
    toolbar->addAction(newAction);

    auto saveAction = new QAction(toolbar);
    saveAction->setIcon(QIcon(":/icons/save.png"));
    saveAction->setToolTip(config->getStr("action", "save", "保存"));
    toolbar->addAction(saveAction);

    auto saveAsAction = new QAction(toolbar);
    saveAsAction->setIcon(QIcon(":/icons/saveas.png"));
    saveAsAction->setToolTip(config->getStr("action", "saveas", "另存为"));
    toolbar->addAction(saveAsAction);

    auto printAction = new QAction(toolbar);
    printAction->setIcon(QIcon(":/icons/print.png"));
    printAction->setToolTip(config->getStr("action", "print", "截图"));
    toolbar->addAction(printAction);

    toolbar->addSeparator();

    auto deleteAction = new QAction(toolbar);
    deleteAction->setIcon(QIcon(":/icons/delete.png"));
    deleteAction->setToolTip(config->getStr("action", "delete", "删除卡组"));
    toolbar->addAction(deleteAction);

    toolbar->addSeparator();

    abortAction = new QAction(toolbar);
    abortAction->setIcon(QIcon(":/icons/abort.png"));
    abortAction->setToolTip(config->getStr("action", "abort", "中止"));
    abortAction->setEnabled(false);
    toolbar->addAction(abortAction);

    toolbar->addSeparator();

    auto hideButton = new QToolButton;
    hideButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    auto hideAction = new QAction("Side", hideButton);
    hideAction->setIcon(QIcon(":/icons/side.png"));

    hideButton->addAction(hideAction);
    hideButton->setDefaultAction(hideAction);
    toolbar->addWidget(hideButton);

    auto spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolbar->addWidget(spacer);

    toolbar->addSeparator();

    auto sortAction = new QAction(toolbar);
    sortAction->setIcon(QIcon(":/icons/sort.png"));
    sortAction->setToolTip(config->getStr("action", "sort", "排序"));
    toolbar->addAction(sortAction);

    auto shuffleAction = new QAction(toolbar);
    shuffleAction->setIcon(QIcon(":/icons/shuffle.png"));
    shuffleAction->setToolTip(config->getStr("action", "shuffle", "打乱"));
    toolbar->addAction(shuffleAction);


    auto clearAction = new QAction(toolbar);
    clearAction->setIcon(QIcon(":/icons/clear.png"));
    clearAction->setToolTip(config->getStr("action", "clear", "清空"));
    toolbar->addAction(clearAction);

    toolbar->addSeparator();

    auto homeAction = new QAction(toolbar);
    homeAction->setIcon(QIcon(":/icons/home.png"));
    homeAction->setToolTip(config->getStr("action", "home", "主页"));
    toolbar->addAction(homeAction);

    auto helpAction = new QAction(toolbar);
    helpAction->setIcon(QIcon(":/icons/help.png"));
    helpAction->setToolTip(config->getStr("action", "help", "帮助"));
    toolbar->addAction(helpAction);

    vbox->addWidget(toolbar);


    hbox->addWidget(t);
    hbox->addWidget(mt);
    vbox->addLayout(hbox);
    vbox->addWidget(mainDeck, 4);

    connect(mainDeck, &DeckWidget::sizeChanged, t, &DeckSizeLabel::changeSize);
    connect(mainDeck, &DeckWidget::deckChanged, mt, &MainDeckLabel::deckChanged);
    connect(mainDeck, &DeckWidget::currentIdChanged, this, &DeckView::currentIdChanged);

    extraDeck = new ExtraDeckWidget(nullptr);
    sideDeck = new DeckWidget(nullptr, 1, 10);

    t = new DeckSizeLabel(config->getStr("label", "extra", "额外卡组"));
    auto et = new ExtraDeckLabel;
    hbox = new QHBoxLayout;
    hbox->addWidget(t);
    hbox->addWidget(et);
    vbox->addLayout(hbox);
    connect(extraDeck, &DeckWidget::sizeChanged, t, &DeckSizeLabel::changeSize);
    connect(extraDeck, &DeckWidget::deckChanged, et, &ExtraDeckLabel::deckChanged);
    connect(extraDeck, &DeckWidget::currentIdChanged, this, &DeckView::currentIdChanged);
    vbox->addWidget(extraDeck, 1);

    st = new DeckSizeLabel(config->getStr("label", "side", "副卡组"));
    vbox->addWidget(st);
    vbox->addWidget(sideDeck, 1);

    connect(sideDeck, &DeckWidget::sizeChanged, st, &DeckSizeLabel::changeSize);
    connect(sideDeck, &DeckWidget::currentIdChanged, this, &DeckView::currentIdChanged);

    auto extFilter = [&](int id) {
        int sum = 0;
        sum += mainDeck->countCard(id);
        sum += extraDeck->countCard(id);
        sum += sideDeck->countCard(id);
        return sum < LimitCards::getLimit(id);
    };

    mainDeck->extFilter = extFilter;
    extraDeck->extFilter = extFilter;
    sideDeck->extFilter = extFilter;

    auto snapshotMaker = [&]() {
        makeSnapshot();
    };

    mainDeck->makeSnapShot = snapshotMaker;
    extraDeck->makeSnapShot = snapshotMaker;
    sideDeck->makeSnapShot = snapshotMaker;

    connect(sortAction, &QAction::triggered, this, &DeckView::sort);
    connect(clearAction, &QAction::triggered, this, &DeckView::clearDeck);
    connect(helpAction, &QAction::triggered, this, &DeckView::help);
    connect(mainDeck, &DeckWidget::clickId, this, &DeckView::idClicked);
    connect(extraDeck, &DeckWidget::clickId, this, &DeckView::idClicked);
    connect(sideDeck, &DeckWidget::clickId, this, &DeckView::idClicked);

    connect(shuffleAction, &QAction::triggered, mainDeck, &DeckWidget::shuffle);

    connect(undoAction, &QAction::triggered, this, &DeckView::undo);
    connect(redoAction, &QAction::triggered, this, &DeckView::redo);
    connect(saveAction, &QAction::triggered, this, &DeckView::saveSlot);
    connect(saveAsAction, &QAction::triggered, this, &DeckView::save);
    connect(newAction, &QAction::triggered, this, &DeckView::newDeck);
    connect(deleteAction, &QAction::triggered, this, &DeckView::deleteDeck);
    connect(abortAction, &QAction::triggered, this, &DeckView::abort);
    connect(homeAction, &QAction::triggered, this, &DeckView::home);
    connect(printAction, &QAction::triggered, this, &DeckView::print);
    connect(hideAction, &QAction::triggered, this, &DeckView::hideSide);

    connect(mainDeck, &DeckWidget::details, this, &DeckView::details);
    connect(extraDeck, &DeckWidget::details, this, &DeckView::details);
    connect(sideDeck, &DeckWidget::details, this, &DeckView::details);

    qRegisterMetaType<Dst>();

    setLayout(vbox);
    updateButtons();
}

void DeckView::loadDeck(QString lines, QString _name, bool local)
{
    if(waiting)
    {
        return;
    }
    int load = ++currentLoad;
    makeSnapshot(false);

    mainDeck->clearDeck();
    extraDeck->clearDeck();
    sideDeck->clearDeck();


    deckStatus.name = _name;
    deckStatus.isLocal = local;
    deckStatus.modified = !local;

    waiting = true;
    setReady(false);
    auto thread = new ItemThread(load, lines, this);
    connect(thread, &ItemThread::finishLoad, this, &DeckView::loadFinished);
    connect(thread, &ItemThread::finished, thread, &ItemThread::deleteLater);
    connect(this, &DeckView::destroyed, thread, &ItemThread::quit);
    thread->start();
}

void DeckView::sort()
{
    makeSnapshot();
    mainDeck->sort();
    extraDeck->sort();
    sideDeck->sort();
}


void DeckView::saveDeck(QString path)
{
    QFile file(path);
    if(file.open(QFile::WriteOnly | QFile::Text))
    {
        QTextStream out(&file);
        out << "#create by ...\n#main\n";
        foreach(auto &item, mainDeck->getDeck())
        {
            out << item.getId() << "\n";
        }
        out << "#extra\n";
        foreach(auto &item, extraDeck->getDeck())
        {
            out << item.getId() << "\n";
        }
        out << "!side\n";
        foreach(auto &item, sideDeck->getDeck())
        {
            out << item.getId() << "\n";
        }
        file.close();
        deckStatus.modified = false;
        deckStatus.isLocal = true;
        deckStatus.name = QFileInfo(file).completeBaseName();
        setStatus();
    }
}

void DeckView::help()
{
    QString helpStr = config->getHelpStr();
    QMessageBox::information(nullptr, config->getStr("action", "help", "帮助"), helpStr);
}

void DeckView::home()
{
    QDesktopServices::openUrl(QUrl("https://github.com/yjqww6/deckmanager"));
}

void DeckView::print()
{
    int y = toolbar->mapToParent(QPoint(0, toolbar->height())).y();
    QPixmap pixmap = QApplication::primaryScreen()->grabWindow(this->winId(), 0, y, width(), height() - y);
    QString filename = QFileDialog::getSaveFileName(this, "Save", "", "*.png");
    if(!filename.isNull())
    {
        if(!filename.endsWith(".png", Qt::CaseInsensitive))
        {
            filename += ".png";
        }
        pixmap.save(filename, "png");
    }
}

void DeckView::hideSide()
{
    if(sideHidden)
    {
        st->show();
        sideDeck->show();
        sideHidden = false;
    }
    else
    {
        st->hide();
        sideDeck->hide();
        sideHidden = true;
    }
}

void DeckView::makeSnapshot(bool mod)
{
    if(snapshots.size() > 50)
    {
        snapshots.pop_back();
    }
    redoSnapshots.clear();
    snapshots.push_front(std::move(currentSnapshot()));

    if(mod)
    {
        deckStatus.modified = true;
    }
    updateButtons();
    setStatus();
}

void DeckView::undo()
{
    if(snapshots.size() >= 1)
    {
        if(redoSnapshots.size() > 50)
        {
            redoSnapshots.pop_back();
        }

        redoSnapshots.push_front(std::move(currentSnapshot()));
        restoreSnapshot(snapshots.front());
        snapshots.pop_front();
        update();
        updateButtons();
        setStatus();
    }
}

void DeckView::redo()
{
    if(redoSnapshots.size() >= 1)
    {
        if(snapshots.size() > 50)
        {
            snapshots.pop_back();
        }

        snapshots.push_front(std::move(currentSnapshot()));
        restoreSnapshot(redoSnapshots.front());
        redoSnapshots.pop_front();
        update();
        updateButtons();
        setStatus();
    }
}

void DeckView::newDeck()
{
    makeSnapshot(false);
    deckStatus.name = "";
    deckStatus.isLocal = false;
    deckStatus.modified = false;
    mainDeck->clearDeck();
    extraDeck->clearDeck();
    sideDeck->clearDeck();
    mainDeck->update();
    extraDeck->update();
    sideDeck->update();
    setStatus();
}

void DeckView::clearDeck()
{
    makeSnapshot();
    mainDeck->clearDeck();
    extraDeck->clearDeck();
    sideDeck->clearDeck();
    mainDeck->update();
    extraDeck->update();
    sideDeck->update();
    setStatus();
}

static void toCards( QList<CardItem>& items, QVector<int> &shot)
{
    QList<CardItem> temp;
    temp.reserve(shot.size());
    foreach(int id, shot)
    {
        temp.append(CardItem(id));
    }
    items.swap(temp);
}

void DeckView::restoreSnapshot(SnapShot &snap)
{
    toCards(mainDeck->getDeck(), snap.shot[0]);
    toCards(extraDeck->getDeck(), snap.shot[1]);
    toCards(sideDeck->getDeck(), snap.shot[2]);
    deckStatus = snap.deckStatus;
}

static void toShot(QVector<int> &shot, QList<CardItem>& items)
{
    shot.reserve(items.size());
    foreach(auto &item, items)
    {
        shot.append(item.getId());
    }
}

void DeckView::updateButtons()
{
    if(snapshots.size() > 0)
    {
        undoAction->setEnabled(true);
    }
    else
    {
        undoAction->setEnabled(false);
    }
    if(redoSnapshots.size() > 0)
    {
        redoAction->setEnabled(true);
    }
    else
    {
        redoAction->setEnabled(false);
    }
}

void DeckView::saveSlot()
{
    if(deckStatus.isLocal)
    {
        saveDeck("deck/" + deckStatus.name + ".ydk");
        deckStatus.modified = false;
    }
    else
    {
        emit save();
    }
}

void DeckView::deleteDeck()
{
    if(deckStatus.isLocal)
    {
        if(QMessageBox::question(nullptr, config->getStr("label", "warning", "警告"),
                                 config->getStr("label", "delete_p", "是否要删除卡组:") + deckStatus.name + "?",
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
            == QMessageBox::Yes)
        {
            QFile file("deck/" + deckStatus.name + ".ydk");
            if(file.remove())
            {
                emit refreshLocals();
            }
        }
    }
}

DeckView::SnapShot DeckView::currentSnapshot()
{
    SnapShot snap;
    toShot(snap.shot[0], mainDeck->getDeck());
    toShot(snap.shot[1], extraDeck->getDeck());
    toShot(snap.shot[2], sideDeck->getDeck());
    snap.deckStatus = deckStatus;
    return std::move(snap);
}

void DeckView::loadFinished(int load, Dst ms, Dst es, Dst ss)
{
    waiting = false;
    if(load == currentLoad)
    {
        mainDeck->getDeck().swap(*ms.data());
        extraDeck->getDeck().swap(*es.data());
        sideDeck->getDeck().swap(*ss.data());

        setStatus();
        mainDeck->update();
        extraDeck->update();
        sideDeck->update();
        setReady(true);
    }
}


QSharedPointer<Card> ItemThread::loadNewCard(int id)
{
    auto it = parent->map.find(id);
    if(it != parent->map.end())
    {
        return CardPool::getCard(it.value());
    }
    QEventLoop loop;
    QString name;
    Remote remote;
    loop.connect(&remote, &Remote::cardName, [&](QString text) {
        name = text;
    });
    loop.connect(&remote, static_cast<void (Remote::*)()>(&Remote::finished), &loop, &QEventLoop::quit);
    remote.getName(id);
    loop.exec();
    auto card = CardPool::getNewCard(name, config->waitForPass);
    if(card)
    {
        parent->map.insert(id, card->id);
    }
    return card;
}

void ItemThread::run()
{
    QTextStream in(&lines);
    bool side = false;

    for(QString line = in.readLine(); !line.isNull(); line = in.readLine())
    {
        if(load != parent->currentLoad || !parent->waiting)
        {
            return;
        }
        if(line.length() > 0)
        {
            if(line[0] == '#')
            {
                continue;
            }
            else if(line[0] == '!')
            {
                side = true;
                continue;
            }
            else
            {
                bool ok = true;
                int id = line.toInt(&ok);

                QSharedPointer<Card> card;
                if(ok)
                {
                    card = CardPool::getCard(id);
                    if(!card && config->convertPass && id >= 10000)
                    {
                        card = loadNewCard(id);
                    }
                }
                else
                {
                    card = CardPool::getNewCard(line, config->waitForPass);
                }

                if(load != parent->currentLoad || !parent->waiting)
                {
                    return;
                }

                if(!card)
                {
                    continue;
                }
                else
                {
                    id = card->id;
                    if(side)
                    {
                        ss->append(CardItem(id));
                    }
                    else
                    {
                        if(card->inExtra())
                        {
                            es->append(CardItem(id));
                        }
                        else
                        {
                            ms->append(CardItem(id));
                        }
                    }
                }
            }
        }
    }
    emit finishLoad(load, ms, es, ss);
}
