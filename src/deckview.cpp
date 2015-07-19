#include "deckview.h"
#include "limitcards.h"
#include "config.h"
#include "range.h"
#include "scriptview.h"
#include "remote.h"
#include <QDebug>
#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QResizeEvent>
#include <QUrl>
#include <QMenu>
#include <QScreen>
#include <QToolButton>

DeckModel& DeckView::addModel()
{
    models.append(QSharedPointer<DeckModel>::create());
    auto &model = models.back();
    int i = tabbar->addTab(model->deckStatus.name);
    tabbar->setTabData(i, model->id);
    connect(model.data(), &DeckModel::refresh, this, &DeckView::modelRefresh);
    connect(model.data(), &DeckModel::ready, this, &DeckView::modelReady);
    return *model;

}

void DeckView::modelRefresh(int modelId)
{
    if(currentModel)
    {
        if(currentModel->id == modelId)
        {
            refresh();
        }
        else
        {
            QString stat;
            foreach(auto it, models)
            {
                if(it->id == modelId)
                {
                    stat = it->status();
                    break;
                }
            }

            int i = getTabIndexById(modelId);
            if(i != -1)
            {
                tabbar->setTabText(i, stat);
            }
        }
    }
}

void DeckView::modelReady(int modelId, bool b)
{
    if(currentModel)
    {
        if(currentModel->id == modelId)
        {
            setReady(b);
        }
    }
}

DeckModel& DeckView::getCurrentModel()
{
    if(currentModel)
    {
        return *currentModel;
    }
    else
    {
        static DeckModel nullModel;
        return nullModel;
    }
}

void DeckView::switchTab(int i)
{
    int id = tabbar->tabData(i).toInt();
    foreach(auto &p, models)
    {
        if(p->id == id)
        {
            currentModel = p;
            mainDeck->setDeck(p->mainDeck);
            extraDeck->setDeck(p->extraDeck);
            sideDeck->setDeck(p->sideDeck);
        }
    }
    refresh();
}

int DeckView::getTabIndexById(int id)
{
    for(int i : range(tabbar->count()))
    {
        int tabId = tabbar->tabData(i).toInt();
        if(tabId == id)
        {
            return i;
        }
    }
    return -1;
}

void DeckView::newTab()
{
    auto &model = addModel();
    int i = getTabIndexById(model.id);
    if(i == -1)
    {
        return;
    }
    tabbar->setCurrentIndex(i);
    refresh();
}

void DeckView::closeTab(int index)
{
    if(tabbar->count() <= 1)
    {
        addModel();
    }
    int id = tabbar->tabData(index).toInt();

    for(int i : range(models.size()))
    {
        if(models[i]->id == id)
        {
            if(models[i]->deckStatus.modified)
            {
                if(QMessageBox::question(nullptr,
                                        config->getStr("label", "warning", "警告"),
                                        config->getStr("label", "close", "未保存,是否关闭?"),
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
                        == QMessageBox::No)
                {
                    return;
                }
            }
            models.removeAt(i);
            break;
        }
    }

    tabbar->removeTab(index);
}

DeckView::DeckView(QWidget *parent, QTabBar *_tabbar)
    : QWidget(parent), sideHidden(false)
{
    tabbar = _tabbar;

    int fh = tabbar->fontMetrics().height() + 5;
    tabbar->setStyleSheet("QTabBar::tab{height: "
                          + QString::number(fh) +
                          "px; color: black; font-size: 12px;"
                          "min-width: 120px; height: 24px}"
                          "QTabBar{background: rgba(255, 255, 255, 200)}");
    tabbar->setElideMode(Qt::ElideMiddle);
    tabbar->setMovable(true);

    toolbar = new QToolBar;
    toolbar->setStyleSheet("QToolTip{color: black; font-size: 12px}");

    undoAction = new QAction(toolbar);
    undoAction->setIcon(QIcon(":/icons/undo.png"));
    undoAction->setToolTip(config->getStr("action", "undo", "撤销"));
    toolbar->addAction(undoAction);

    redoAction = new QAction(toolbar);
    redoAction->setIcon(QIcon(":/icons/redo.png"));
    redoAction->setToolTip(config->getStr("action", "redo", "重做"));
    toolbar->addAction(redoAction);

    toolbar->addSeparator();

    auto saveAction = new QAction(toolbar);
    saveAction->setIcon(QIcon(":/icons/save.png"));
    saveAction->setToolTip(config->getStr("action", "save", "保存"));
    toolbar->addAction(saveAction);

    auto saveAsAction = new QAction(toolbar);
    saveAsAction->setIcon(QIcon(":/icons/saveas.png"));
    saveAsAction->setToolTip(config->getStr("action", "saveas", "另存为"));
    toolbar->addAction(saveAsAction);


    auto menu = new QMenu;

    auto printAction = new QAction(config->getStr("action", "print", "截图"), menu);
    menu->addAction(printAction);

    auto textAction1 = new QAction(config->getStr("action", "text", "文字卡表") + " A", menu);
    menu->addAction(textAction1);
    auto textAction2 = new QAction(config->getStr("action", "text", "文字卡表") + " [A]", menu);
    menu->addAction(textAction2);
    auto textAction3 = new QAction(config->getStr("action", "text", "文字卡表") + " n A", menu);
    menu->addAction(textAction3);

    auto toolButton = new QToolButton();
    toolButton->setIcon(QIcon(":/icons/print.png"));
    toolButton->setMenu(menu);
    toolButton->setPopupMode(QToolButton::InstantPopup);
    toolbar->addWidget(toolButton);

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

    auto hideAction = new QAction(toolbar);
    hideAction->setToolTip(config->getStr("action", "hide", "隐藏副卡组"));
    hideAction->setIcon(QIcon(":/icons/side.png"));
    toolbar->addAction(hideAction);

    auto overlapAction = new QAction(toolbar);
    overlapAction->setToolTip(config->getStr("action", "overlap", "垂直方向重叠"));
    overlapAction->setIcon(QIcon(":/icons/overlap.png"));
    toolbar->addAction(overlapAction);

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


    auto &model = addModel();
    currentModel = models.back();

    mainDeck = new DeckWidget(nullptr, 4, 10, model.mainDeck);
    mainDeck->overlapV = true;
    auto notExtraFilter = [](quint32 id)
    {
        return call_with_def([](Card &card) {
            return !card.inExtra();
        }, false, cardPool->getCard(id));
    };

    mainDeck->filter = notExtraFilter;
    auto t1 = new DeckSizeLabel(config->getStr("label", "main", "主卡组"));
    auto mt = new MainDeckLabel;

    extraDeck = new DeckWidget(nullptr, 1, 10, model.extraDeck);
    auto extraFilter = [](quint32 id)
    {
        return call_with_def([](Card &card) {
            return card.inExtra();
        }, false, cardPool->getCard(id));
    };
    extraDeck->filter = extraFilter;
    sideDeck = new DeckWidget(nullptr, 1, 10, model.sideDeck);

    auto t2 = new DeckSizeLabel(config->getStr("label", "extra", "额外卡组"));
    auto et = new ExtraDeckLabel;

    st = new DeckSizeLabel(config->getStr("label", "side", "副卡组"));

    auto extFilter = [this](quint32 id) {
        int sum = 0;
        sum += mainDeck->countCard(id);
        sum += extraDeck->countCard(id);
        sum += sideDeck->countCard(id);
        return sum < limitCards->getLimit(id);
    };

    mainDeck->extFilter = extFilter;
    extraDeck->extFilter = extFilter;
    sideDeck->extFilter = extFilter;

    auto snapshotMaker = [this]() {
        makeSnapShot();
    };

    mainDeck->makeSnapShot = snapshotMaker;
    extraDeck->makeSnapShot = snapshotMaker;
    sideDeck->makeSnapShot = snapshotMaker;

    connect(mainDeck, &DeckWidget::sizeChanged, t1, &DeckSizeLabel::changeSize);
    connect(mainDeck, &DeckWidget::deckChanged, mt, &MainDeckLabel::deckChanged);
    connect(mainDeck, &DeckWidget::currentIdChanged, this, &DeckView::currentIdChanged);

    connect(extraDeck, &DeckWidget::sizeChanged, t2, &DeckSizeLabel::changeSize);
    connect(extraDeck, &DeckWidget::deckChanged, et, &ExtraDeckLabel::deckChanged);
    connect(extraDeck, &DeckWidget::currentIdChanged, this, &DeckView::currentIdChanged);
    connect(sideDeck, &DeckWidget::sizeChanged, st, &DeckSizeLabel::changeSize);
    connect(sideDeck, &DeckWidget::currentIdChanged, this, &DeckView::currentIdChanged);

    connect(sortAction, &QAction::triggered, this, &DeckView::sort);
    connect(clearAction, &QAction::triggered, this, &DeckView::clearDeck);
    connect(helpAction, &QAction::triggered, this, &DeckView::help);
    connect(mainDeck, &DeckWidget::clickId, this, &DeckView::clickId);
    connect(extraDeck, &DeckWidget::clickId, this, &DeckView::clickId);
    connect(sideDeck, &DeckWidget::clickId, this, &DeckView::clickId);

    connect(shuffleAction, &QAction::triggered, this, &DeckView::shuffle);

    connect(undoAction, &QAction::triggered, this, &DeckView::undo);
    connect(redoAction, &QAction::triggered, this, &DeckView::redo);
    connect(saveAction, &QAction::triggered, this, &DeckView::saveSlot);
    connect(saveAsAction, &QAction::triggered, [this]() {
        emit save(getCurrentModel().deckStatus.name);
    });
    connect(deleteAction, &QAction::triggered, this, &DeckView::deleteDeck);
    connect(abortAction, &QAction::triggered, this, &DeckView::abort);
    connect(homeAction, &QAction::triggered, this, &DeckView::home);
    connect(printAction, &QAction::triggered, this, &DeckView::print);
    connect(hideAction, &QAction::triggered, this, &DeckView::hideSide);
    connect(overlapAction, &QAction::triggered, [=]() {
       mainDeck->overlapV = !mainDeck->overlapV;
       mainDeck->update();
    });

    connect(textAction1, &QAction::triggered, [=]() {
       emit deckText(&getCurrentModel(), sideHidden, ScriptView::NORMAL);
    });

    connect(textAction2, &QAction::triggered, [=]() {
       emit deckText(&getCurrentModel(), sideHidden, ScriptView::BRACKET);
    });

    connect(textAction3, &QAction::triggered, [=]() {
       emit deckText(&getCurrentModel(), sideHidden, ScriptView::COUNT);
    });

    connect(mainDeck, &DeckWidget::details, this, &DeckView::details);
    connect(extraDeck, &DeckWidget::details, this, &DeckView::details);
    connect(sideDeck, &DeckWidget::details, this, &DeckView::details);

    connect(tabbar, &QTabBar::currentChanged, this, &DeckView::switchTab);
    connect(tabbar, &QTabBar::tabCloseRequested, this, &DeckView::closeTab);

    QString ss;

    if(config->bg)
    {

        ss = "QWidget{color: white; font-size: 16px}";
    }
    else
    {
        ss = "QWidget{font-size: 16px}";
    }

    mainDeck->setStyleSheet(ss);
    extraDeck->setStyleSheet(ss);
    sideDeck->setStyleSheet(ss);
    t1->setStyleSheet(ss);
    t2->setStyleSheet(ss);
    mt->setStyleSheet(ss);
    et->setStyleSheet(ss);
    st->setStyleSheet(ss);

    auto vbox = new QVBoxLayout;
    auto hbox = new QHBoxLayout;
    vbox->setContentsMargins(5, 0, 5, 5);
    vbox->setSpacing(2);

    tabbar->setTabsClosable(true);
    vbox->addWidget(toolbar);

    hbox->addWidget(t1);
    hbox->addWidget(mt);
    vbox->addLayout(hbox);
    vbox->addWidget(mainDeck, 4);

    hbox = new QHBoxLayout;
    hbox->addWidget(t2);
    hbox->addWidget(et);
    vbox->addLayout(hbox);
    vbox->addWidget(extraDeck, 1);
    vbox->addWidget(st);
    vbox->addWidget(sideDeck, 1);
    setLayout(vbox);

    refresh();
}

void DeckView::loadRemoteDeck(QString id, QString name, bool newTab)
{
    if(newTab || (!getCurrentModel().fresh && config->newTab))
    {
        addModel();
        tabbar->setCurrentIndex(tabbar->count() - 1);
    }
    getCurrentModel().loadRemoteDeck(id, name);
}

void DeckView::loadDeck(QString lines, QString _name, bool local, bool newTab)
{
    if(newTab || (!getCurrentModel().fresh && config->newTab))
    {
        addModel();
        tabbar->setCurrentIndex(tabbar->count() - 1);
    }
    getCurrentModel().loadDeck(lines, _name, local);
}

void DeckView::sort()
{
    getCurrentModel().sort();
    refresh();
}

void DeckView::shuffle()
{
    getCurrentModel().shuffle();
    refresh();
}


void DeckView::saveDeck(QString path)
{
    getCurrentModel().saveDeck(path);
    refresh();
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

void DeckView::makeSnapShot(bool mod)
{
    getCurrentModel().makeSnapShot(mod);
    refresh();
}

void DeckView::undo()
{
    getCurrentModel().undo();
    refresh();
}

void DeckView::redo()
{
    getCurrentModel().redo();
    refresh();
}

void DeckView::clearDeck()
{
    getCurrentModel().clear();
    refresh();
}

void DeckView::setStatus()
{
    auto &model = getCurrentModel();
    QString prefix = "deckmanager - by qww6 ";
    QString stat = model.status();
    int i = getTabIndexById(model.id);
    if(i != -1)
    {
        tabbar->setTabText(i, stat);
    }

    emit statusChanged(prefix + stat);
}

void DeckView::refresh()
{
    auto &model = getCurrentModel();
    mainDeck->update();
    extraDeck->update();
    sideDeck->update();

    undoAction->setEnabled(model.snapshots.size() > 0);
    redoAction->setEnabled(model.redoSnapshots.size() > 0);
    setReady(!model.waiting);
    setStatus();
}

void DeckView::saveSlot()
{
    auto &model = getCurrentModel();
    if(model.deckStatus.isLocal)
    {
        saveDeck("deck/" + model.deckStatus.name + ".ydk");
        model.deckStatus.modified = false;
    }
    else
    {
        emit save(model.deckStatus.name);
    }
}

void DeckView::deleteDeck()
{
    auto &model = getCurrentModel();
    if(model.deckStatus.isLocal)
    {
        if(QMessageBox::question(nullptr, config->getStr("label", "warning", "警告"),
                                 config->getStr("label", "delete_p", "是否要删除卡组:") + model.deckStatus.name + "?",
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
            == QMessageBox::Yes)
        {
            QFile file("deck/" + model.deckStatus.name + ".ydk");
            if(file.remove())
            {
                emit refreshLocals();
            }
        }
    }
}
