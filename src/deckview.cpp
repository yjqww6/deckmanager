#include "deckview.h"
#include "limitcards.h"
#include "configmanager.h"
#include "range.h"
#include "scriptview.h"
#include "signaltower.h"
#include <QDebug>
#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QResizeEvent>
#include <QUrl>
#include <QMenu>
#include <QScreen>
#include <QToolButton>
#include <QDialogButtonBox>

DeckModel& DeckView::addModel()
{
    m_models.append(QSharedPointer<DeckModel>::create());
    auto &model = m_models.back();
    int i = m_tabbar->addTab(model->m_deckStatus.name);
    m_tabbar->setTabData(i, model->m_id);
    connect(model.data(), &DeckModel::refresh, this, &DeckView::modelRefresh);
    connect(model.data(), &DeckModel::ready, this, &DeckView::modelReady);
    return *model;

}

void DeckView::modelRefresh(int modelId)
{
    if(m_currentModel)
    {
        if(m_currentModel->m_id == modelId)
        {
            refresh();
        }
        else
        {
            QString stat;
            foreach(auto it, m_models)
            {
                if(it->m_id == modelId)
                {
                    stat = it->status();
                    break;
                }
            }

            int i = getTabIndexById(modelId);
            if(i != -1)
            {
                m_tabbar->setTabText(i, stat);
            }
        }
    }
}

void DeckView::modelReady(int modelId, bool b)
{
    if(m_currentModel)
    {
        if(m_currentModel->m_id == modelId)
        {
            setReady(b);
        }
    }
}

DeckModel& DeckView::getCurrentModel()
{
    if(m_currentModel)
    {
        return *m_currentModel;
    }
    else
    {
        static DeckModel nullModel;
        return nullModel;
    }
}

void DeckView::switchTab(int i)
{
    int id = m_tabbar->tabData(i).toInt();
    foreach(auto &p, m_models)
    {
        if(p->m_id == id)
        {
            m_currentModel = p;
            m_mainDeck->setDeck(p->m_mainDeck);
            m_extraDeck->setDeck(p->m_extraDeck);
            m_sideDeck->setDeck(p->m_sideDeck);
        }
    }
    refresh();
}

int DeckView::getTabIndexById(int id)
{
    for(int i : range(m_tabbar->count()))
    {
        int tabId = m_tabbar->tabData(i).toInt();
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
    int i = getTabIndexById(model.m_id);
    if(i == -1)
    {
        return;
    }
    m_tabbar->setCurrentIndex(i);
    refresh();
}

void DeckView::closeTab(int index)
{
    if(m_tabbar->count() <= 1)
    {
        addModel();
    }
    int id = m_tabbar->tabData(index).toInt();

    for(int i : range(m_models.size()))
    {
        if(m_models[i]->m_id == id)
        {
            if(m_models[i]->m_deckStatus.modified)
            {
                if(QMessageBox::question(nullptr,
                                        ConfigManager::inst().getStr("label", "warning", "警告"),
                                        ConfigManager::inst().getStr("label", "close", "未保存,是否关闭?"),
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
                        == QMessageBox::No)
                {
                    return;
                }
            }
            m_models.removeAt(i);
            break;
        }
    }

    m_tabbar->removeTab(index);
}

DeckView::DeckView(QWidget *parent, QTabBar *_tabbar)
    : QWidget(parent), m_sideHidden(false)
{
    m_tabbar = _tabbar;

    int fh = m_tabbar->fontMetrics().height() + 5;
    m_tabbar->setStyleSheet("QTabBar::tab{height: "
                          + QString::number(fh) +
                          "px; color: black; font-size: 12px;"
                          "min-width: 120px; height: 24px}"
                          "QTabBar{background: rgba(255, 255, 255, 200)}");
    m_tabbar->setElideMode(Qt::ElideMiddle);
    m_tabbar->setMovable(true);

    m_toolbar = new QToolBar;
    m_toolbar->setStyleSheet("QToolTip{color: black; font-size: 12px}");

    auto action = [&](const char* path, const char* name, const char* str)
    {
        auto act = new QAction(m_toolbar);
        act->setIcon(QIcon(path));
        act->setToolTip(ConfigManager::inst().getStr("action", name, str));
        m_toolbar->addAction(act);
        return act;
    };

    m_undoAction = action(":/icons/undo.png", "undo", "撤销");
    m_redoAction = action(":/icons/redo.png", "redo", "重做");

    m_toolbar->addSeparator();
    auto saveAction = action(":/icons/save.png", "save", "保存");
    auto saveAsAction = action(":/icons/saveas.png", "saveas", "另存为");

    auto menu = new QMenu;
    auto printAction = new QAction(ConfigManager::inst().getStr("action", "print", "截图"), menu);
    menu->addAction(printAction);
    auto textAction1 = new QAction(ConfigManager::inst().getStr("action", "text", "文字卡表") + " A", menu);
    menu->addAction(textAction1);
    auto textAction2 = new QAction(ConfigManager::inst().getStr("action", "text", "文字卡表") + " [A]", menu);
    menu->addAction(textAction2);
    auto textAction3 = new QAction(ConfigManager::inst().getStr("action", "text", "文字卡表") + " n A", menu);
    menu->addAction(textAction3);

    auto toolButton = new QToolButton();
    toolButton->setIcon(QIcon(":/icons/print.png"));
    toolButton->setMenu(menu);
    toolButton->setPopupMode(QToolButton::InstantPopup);
    m_toolbar->addWidget(toolButton);

    m_toolbar->addSeparator();

    auto deleteAction = action(":/icons/delete.png", "delete", "删除卡组");

    m_toolbar->addSeparator();

    m_abortAction = action(":/icons/abort.png", "abort", "中止");
    m_abortAction->setEnabled(false);

    auto hideAction = action(":/icons/side.png", "hide", "隐藏副卡组");
    auto overlapAction = action(":/icons/overlap.png", "overlap", "垂直方向重叠");

    auto spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_toolbar->addWidget(spacer);

    m_toolbar->addSeparator();

    auto sortAction = action(":/icons/sort.png", "sort", "排序");
    auto shuffleAction = action(":/icons/shuffle.png", "shuffle", "打乱");
    auto clearAction = action(":/icons/clear.png", "clear", "清空");

    m_toolbar->addSeparator();
    auto scriptAction = action(":/icons/script.png", "Scheme", "Scheme");

    auto homeAction = action(":/icons/home.png", "home", "主页");
    auto helpAction = action(":/icons/help.png", "help", "帮助");


    auto &model = addModel();
    m_currentModel = m_models.back();

    m_mainDeck = new DeckWidget(nullptr, 4, 10, model.m_mainDeck);
    m_mainDeck->m_overlapV = true;
    auto notExtraFilter = [](quint32 id)
    {
        if(auto ocard = CardManager::inst().getCard(id))
        {
            Card &card = **ocard;
            return !card.inExtra();
        }
        else
        {
            return false;
        }
    };

    m_mainDeck->m_filter = notExtraFilter;
    auto t1 = new DeckSizeLabel(ConfigManager::inst().getStr("label", "main", "主卡组"));
    auto mt = new MainDeckLabel;

    m_extraDeck = new DeckWidget(nullptr, 1, 10, model.m_extraDeck);
    auto extraFilter = [](quint32 id)
    {
        if(auto ocard = CardManager::inst().getCard(id))
        {
            Card &card = **ocard;
            return card.inExtra();
        }
        else
        {
            return false;
        }
    };
    m_extraDeck->m_filter = extraFilter;
    m_sideDeck = new DeckWidget(nullptr, 1, 10, model.m_sideDeck);

    auto t2 = new DeckSizeLabel(ConfigManager::inst().getStr("label", "extra", "额外卡组"));
    auto et = new ExtraDeckLabel;

    st = new DeckSizeLabel(ConfigManager::inst().getStr("label", "side", "副卡组"));

    auto extFilter = [this](quint32 id) {
        int sum = 0;
        sum += m_mainDeck->countCard(id);
        sum += m_extraDeck->countCard(id);
        sum += m_sideDeck->countCard(id);
        return sum < LimitCards::inst().getLimit(id);
    };

    m_mainDeck->m_extFilter = extFilter;
    m_extraDeck->m_extFilter = extFilter;
    m_sideDeck->m_extFilter = extFilter;

    auto snapshotMaker = [this]() {
        makeSnapShot();
    };

    m_mainDeck->m_makeSnapShot = snapshotMaker;
    m_extraDeck->m_makeSnapShot = snapshotMaker;
    m_sideDeck->m_makeSnapShot = snapshotMaker;

    connect(m_mainDeck, &DeckWidget::sizeChanged, t1, &DeckSizeLabel::changeSize);
    connect(m_mainDeck, &DeckWidget::deckChanged, mt, &MainDeckLabel::deckChanged);

    connect(m_extraDeck, &DeckWidget::sizeChanged, t2, &DeckSizeLabel::changeSize);
    connect(m_extraDeck, &DeckWidget::deckChanged, et, &ExtraDeckLabel::deckChanged);
    connect(m_sideDeck, &DeckWidget::sizeChanged, st, &DeckSizeLabel::changeSize);

    connect(sortAction, &QAction::triggered, this, &DeckView::sort);
    connect(clearAction, &QAction::triggered, this, &DeckView::clearDeck);
    connect(helpAction, &QAction::triggered, this, &DeckView::help);

    connect(shuffleAction, &QAction::triggered, this, &DeckView::shuffle);

    connect(m_undoAction, &QAction::triggered, this, &DeckView::undo);
    connect(m_redoAction, &QAction::triggered, this, &DeckView::redo);
    connect(saveAction, &QAction::triggered, this, &DeckView::saveSlot);
    connect(saveAsAction, &QAction::triggered, [this]() {
        emit save(getCurrentModel().m_deckStatus.name);
    });
    connect(deleteAction, &QAction::triggered, this, &DeckView::deleteDeck);
    connect(m_abortAction, &QAction::triggered, this, &DeckView::abort);
    connect(homeAction, &QAction::triggered, this, &DeckView::home);
    connect(printAction, &QAction::triggered, this, &DeckView::print);
    connect(hideAction, &QAction::triggered, this, &DeckView::hideSide);
    connect(overlapAction, &QAction::triggered, [=]() {
        m_mainDeck->m_overlapV = !m_mainDeck->m_overlapV;
        m_mainDeck->update();
    });

    connect(textAction1, &QAction::triggered, [=]() {
        emit deckText(&getCurrentModel(), m_sideHidden, ScriptView::NORMAL);
    });

    connect(textAction2, &QAction::triggered, [=]() {
        emit deckText(&getCurrentModel(), m_sideHidden, ScriptView::BRACKET);
    });

    connect(textAction3, &QAction::triggered, [=]() {
        emit deckText(&getCurrentModel(), m_sideHidden, ScriptView::COUNT);
    });

    auto execDlg = new QDialog(this);
    execDlg->setWindowTitle("Input Scheme Code:");
    auto dlgBox = new QVBoxLayout(execDlg);
    auto execInput = new QPlainTextEdit(execDlg);
    auto buttonBox = new QDialogButtonBox (QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, execDlg);
    dlgBox->addWidget(execInput);
    dlgBox->addWidget(buttonBox);
    execDlg->setLayout(dlgBox);
    connect(buttonBox, &QDialogButtonBox::accepted, execDlg, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, execDlg, &QDialog::reject);
    connect(scriptAction, &QAction::triggered, [=]()
    {
        if(execDlg->exec() != QDialog::Accepted)
        {
            return;
        }
        QString cmd = execInput->toPlainText();
        with_scheme([&]()
        {
            ptr ret = engine->eval(cmd.toUtf8().data());
            if(engine->isException(ret))
            {
                return;
            }
            ptr s = engine->call("format", Sfalse, Sstring("~s"), ret);
            QString str = engine->getString(s);
            SignalTower::inst().schemeDebug(str);
        });
    });

    connect(m_tabbar, &QTabBar::currentChanged, this, &DeckView::switchTab);
    connect(m_tabbar, &QTabBar::tabCloseRequested, this, &DeckView::closeTab);

    QString ss;

    if(ConfigManager::inst().m_bg)
    {

        ss = "QWidget{color: white; font-size: 16px}";
    }
    else
    {
        ss = "QWidget{font-size: 16px}";
    }

    m_mainDeck->setStyleSheet(ss);
    m_extraDeck->setStyleSheet(ss);
    m_sideDeck->setStyleSheet(ss);
    t1->setStyleSheet(ss);
    t2->setStyleSheet(ss);
    mt->setStyleSheet(ss);
    et->setStyleSheet(ss);
    st->setStyleSheet(ss);

    auto vbox = new QVBoxLayout;
    auto hbox = new QHBoxLayout;
    vbox->setContentsMargins(5, 0, 5, 5);
    vbox->setSpacing(2);

    m_tabbar->setTabsClosable(true);
    vbox->addWidget(m_toolbar);

    hbox->addWidget(t1);
    hbox->addWidget(mt);
    vbox->addLayout(hbox);
    vbox->addWidget(m_mainDeck, 4);

    hbox = new QHBoxLayout;
    hbox->addWidget(t2);
    hbox->addWidget(et);
    vbox->addLayout(hbox);
    vbox->addWidget(m_extraDeck, 1);
    vbox->addWidget(st);
    vbox->addWidget(m_sideDeck, 1);
    setLayout(vbox);

    refresh();
}

void DeckView::loadRemoteDeck(QString id, QString name, bool newTab)
{
    if(newTab || (!getCurrentModel().m_fresh && ConfigManager::inst().m_newTab))
    {
        addModel();
        m_tabbar->setCurrentIndex(m_tabbar->count() - 1);
    }
    getCurrentModel().loadRemoteDeck(id, name);
}

void DeckView::loadDeck(QString lines, QString _name, bool local, bool newTab)
{
    if(newTab || (!getCurrentModel().m_fresh && ConfigManager::inst().m_newTab))
    {
        addModel();
        m_tabbar->setCurrentIndex(m_tabbar->count() - 1);
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
    QString helpStr = ConfigManager::inst().getHelpStr();
    QMessageBox msg;
    msg.setTextFormat(Qt::RichText);
    msg.setWindowTitle(ConfigManager::inst().getStr("action", "help", "帮助"));
    msg.setText(helpStr);
    msg.exec();
}

void DeckView::home()
{
    QDesktopServices::openUrl(QUrl("https://github.com/yjqww6/deckmanager"));
}

void DeckView::print()
{
    int y = m_toolbar->mapToParent(QPoint(0, m_toolbar->height())).y();
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
    if(m_sideHidden)
    {
        st->show();
        m_sideDeck->show();
        m_sideHidden = false;
    }
    else
    {
        st->hide();
        m_sideDeck->hide();
        m_sideHidden = true;
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
    int i = getTabIndexById(model.m_id);
    if(i != -1)
    {
        m_tabbar->setTabText(i, stat);
    }

    emit statusChanged(prefix + stat);
}

void DeckView::refresh()
{
    auto &model = getCurrentModel();
    m_mainDeck->update();
    m_extraDeck->update();
    m_sideDeck->update();

    m_undoAction->setEnabled(model.m_snapshots.size() > 0);
    m_redoAction->setEnabled(model.m_redoSnapshots.size() > 0);
    setReady(!model.m_waiting);
    setStatus();
}

void DeckView::saveSlot()
{
    auto &model = getCurrentModel();
    if(model.m_deckStatus.isLocal)
    {
        saveDeck("deck/" + model.m_deckStatus.name + ".ydk");
        model.m_deckStatus.modified = false;
    }
    else
    {
        emit save(model.m_deckStatus.name);
    }
}

void DeckView::deleteDeck()
{
    auto &model = getCurrentModel();
    if(model.m_deckStatus.isLocal)
    {
        if(QMessageBox::question(nullptr, ConfigManager::inst().getStr("label", "warning", "警告"),
                                 ConfigManager::inst().getStr("label", "delete_p", "是否要删除卡组:") + model.m_deckStatus.name + "?",
                QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
            == QMessageBox::Yes)
        {
            QFile file("deck/" + model.m_deckStatus.name + ".ydk");
            if(file.remove())
            {
                emit refreshLocals();
            }
        }
    }
}
