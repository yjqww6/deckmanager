#include "decklist.h"
#include "iconbutton.h"
#include "configmanager.h"
#include "arrange.h"
#include "engine.h"
#include <QDebug>
#include <QUrl>
#include <QComboBox>
#include <QCompleter>

DeckList::DeckList(QWidget *parent)
    : QListWidget(parent), m_menuItem(nullptr)
{
    m_popup = new QMenu(this);
    auto sameAction = new QAction(m_popup);
    auto newTabAction = new QAction(m_popup);
    auto openUrlAction = new QAction(m_popup);
    sameAction->setText(ConfigManager::inst().getStr("action", "same", "同类卡组"));
    newTabAction->setText(ConfigManager::inst().getStr("action", "newtab", "在新标签页打开"));
    openUrlAction->setText(ConfigManager::inst().getStr("action", "openurl", "打开网址"));
    m_popup->addAction(sameAction);
    m_popup->addAction(newTabAction);
    m_popup->addAction(openUrlAction);

    connect(sameAction, &QAction::triggered, this, &DeckList::same);
    connect(newTabAction, &QAction::triggered, this, &DeckList::newTab);
    connect(openUrlAction, &QAction::triggered, this, &DeckList::openURL);
    connect(this, &DeckList::itemClicked, this, &DeckList::onItem);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

DeckList::~DeckList()
{

}

void DeckList::contextMenuEvent(QContextMenuEvent *)
{
    auto item = itemAt(mapFromGlobal(QCursor::pos()));
    if(item)
    {
        m_menuItem = item;
        m_popup->exec(QCursor::pos());
    }
}

void DeckList::same()
{
    auto item = m_menuItem;
    if(item)
    {
        auto vls = item->data(Qt::UserRole).toList();
        QString type = vls[1].toString();
        emit deckType(type);
    }
}

void DeckList::setList(Type::DeckL ls)
{
    clear();
    foreach(auto &it, *ls)
    {
        auto item = new QListWidgetItem;
        item->setText(it.first);
        item->setData(Qt::UserRole, it.second);
        item->setToolTip(it.second[2].toString());
        addItem(item);
    }
}

void DeckList::onItemChanged()
{
    onItem(currentItem());
}

void DeckList::onItem(QListWidgetItem *item)
{
    QVariantList vls = item->data(Qt::UserRole).toList();
    QString id = vls[0].toString();
    emit selectDeck(id, item->text(), false);
}

void DeckList::openURL()
{
    auto item = m_menuItem;
    if(item)
    {
        QString id = item->data(Qt::UserRole).toList()[0].toString();

        const QString& url_str = with_scheme([&]()
        {
            ptr res = engine->call("build-url-open", Sfixnum(id.toUInt()));
            return engine->getString(res);
        });
        QUrl url(url_str);
        QDesktopServices::openUrl(url);
    }
}

void DeckList::newTab()
{
    auto item = m_menuItem;
    if(!item)
    {
        return;
    }
    QVariantList vls = item->data(Qt::UserRole).toList();
    QString id = vls[0].toString();
    emit selectDeck(id, item->text(), true);
}

DeckListView::DeckListView(QWidget *parent)
    : QWidget(parent), m_networking(make_networking()), m_lastConfig(0), m_lastPage(1)
{
    auto vbox = new QVBoxLayout;
    auto grid = new QGridLayout;
    arrange arr("a.ab.b1c", grid);

    grid->setMargin(2);

    auto cata = new QComboBox;
    foreach(auto it, ConfigManager::inst().m_Flts)
    {
        cata->addItem(it.second, it.first);
    }
    cata->addItem("Temp", -1);

    auto typeCata = new QComboBox;
    QStringList words;
    typeCata->addItem("-", 0);
    foreach(auto it, ConfigManager::inst().m_deckTypes)
    {
        typeCata->addItem(it.second, it.first);
        words << it.second;
    }
    auto completer = new QCompleter(words, this);
    typeCata->setEditable(true);
    typeCata->setCompleter(completer);

    auto switchButton = new QPushButton("...");
    switchButton->setMaximumWidth(20);

    auto refreshOrAbortButton = new IconButton(":/icons/refresh.png", ConfigManager::inst().getStr("action", "refresh", "刷新"));
    refreshOrAbortButton->setStyleSheet("QPushButton {border: none;} QPushButton:hover {border-width: 1px;border-style: solid; border-radius: 4px; border-color: white}");
    arr.set('a', cata);
    arr.set('b', typeCata);
    arr.set('c', refreshOrAbortButton);
    arr.set('1', switchButton);
    vbox->addLayout(grid);

    m_decklist = new DeckList;
    vbox->addWidget(m_decklist);

    auto hbox = new QHBoxLayout;
    hbox->setMargin(2);

    auto buttom = new QWidget;

    m_pageBox = new QSpinBox();
    m_pageBox->setMinimum(1);
    m_pageBox->setKeyboardTracking(false);

    auto left = new IconButton(":/icons/left.png");
    auto right = new IconButton(":/icons/right.png");

    hbox->addWidget(m_pageBox);
    hbox->addWidget(left);
    hbox->addWidget(right);
    buttom->setLayout(hbox);

    vbox->addWidget(buttom);

    setLayout(vbox);

    connect(m_pageBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &DeckListView::goPage);

    connect(m_networking.get(), &NetWorking::list, m_decklist, &DeckList::setList, Qt::QueuedConnection);
    connect(m_networking.get(), &NetWorking::ready, this, [=] (bool e) {
        m_pageBox->setEnabled(e);
        left->setEnabled(e);
        right->setEnabled(e);
        if(e)
        {
            refreshOrAbortButton->setIcon(QIcon(":/icons/refresh.png"));
        }
        else
        {
            refreshOrAbortButton->setIcon(QIcon(":/icons/abort.png"));
        }
    }, Qt::QueuedConnection);

    connect(m_decklist, &DeckList::selectDeck, this, &DeckListView::selectDeck);

    connect(refreshOrAbortButton, &IconButton::clicked, [=]()
    {
        if(m_networking->getWaiting())
        {
            m_networking->abort();
            refreshOrAbortButton->setIcon(QIcon(":/icons/refresh.png"));
            m_pageBox->setEnabled(true);
            left->setEnabled(true);
            right->setEnabled(true);
        }
        else
        {
            getList(m_pageBox->value());
            refreshOrAbortButton->setIcon(QIcon(":/icons/abort.png"));
        }
    });

    connect(cata, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
        ConfigManager::inst().m_Flt = cata->itemData(index).toInt();
    });
    connect(typeCata, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
        ConfigManager::inst().m_deckType = typeCata->itemData(index).toInt();
    });


    connect(right, &IconButton::clicked, [=]()
    {
        goPage(m_pageBox->value() + 1);
    });
    connect(left, &IconButton::clicked, [=]()
    {

        goPage(m_pageBox->value() - 1);
    });

    connect(m_decklist, &DeckList::deckType, [=] (QString type) {
        ConfigManager::inst().m_tempConfig = type;
        ConfigManager::inst().m_Flt = -1;
        cata->setCurrentIndex(cata->count() - 1);
        goPage(1);
    });

    connect(switchButton, &QPushButton::clicked, [=] () {
        bool ok;
        QString text = QInputDialog::getText(this, "getlistparam", "", QLineEdit::Normal,
                                             ConfigManager::inst().m_tempConfig, &ok);
        if(ok)
        {
            ConfigManager::inst().m_tempConfig = text;
            ConfigManager::inst().m_Flt = -1;
            cata->setCurrentIndex(cata->count() - 1);
            goPage(1);
        }
    });
}

void DeckListView::setList(Type::DeckL ls)
{
    m_decklist->setList(ls);
}

void DeckListView::getList(int newPage)
{
    m_pageBox->setValue(newPage);
    m_networking->getList(newPage);
}

void DeckListView::goPage(int newPage)
{
    if(newPage > 0)
    {
        getList(newPage);
    }
}
