#include "decklist.h"
#include "iconbutton.h"
#include "config.h"
#include "arrange.h"
#include "engine.h"
#include <QDebug>
#include <QUrl>
#include <QComboBox>
#include <QCompleter>

DeckList::DeckList(QWidget *parent)
    : QListWidget(parent), menuItem(nullptr)
{
    popup = new QMenu(this);
    auto sameAction = new QAction(popup);
    auto newTabAction = new QAction(popup);
    auto openUrlAction = new QAction(popup);
    sameAction->setText(config->getStr("action", "same", "同类卡组"));
    newTabAction->setText(config->getStr("action", "newtab", "在新标签页打开"));
    openUrlAction->setText(config->getStr("action", "openurl", "打开网址"));
    popup->addAction(sameAction);
    popup->addAction(newTabAction);
    popup->addAction(openUrlAction);

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
        menuItem = item;
        popup->exec(QCursor::pos());
    }
}

void DeckList::same()
{
    auto item = menuItem;
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
    auto item = menuItem;
    if(item)
    {
        QString id = item->data(Qt::UserRole).toList()[0].toString();

        const std::string& url_str = with_scheme([&]()
        {
            ptr res = engine->call("build-url-open", Sfixnum(id.toUInt()));
            return engine->getString(res);
        });
        QUrl url(QString::fromUtf8(url_str.c_str()));
        QDesktopServices::openUrl(url);
    }
}

void DeckList::newTab()
{
    auto item = menuItem;
    if(!item)
    {
        return;
    }
    QVariantList vls = item->data(Qt::UserRole).toList();
    QString id = vls[0].toString();
    emit selectDeck(id, item->text(), true);
}

DeckListView::DeckListView(QWidget *parent)
    : QWidget(parent), networking(make_networking()), lastConfig(0), lastPage(1)
{
    auto vbox = new QVBoxLayout;
    auto grid = new QGridLayout;
    arrange arr("a.ab.b1c", grid);

    grid->setMargin(2);

    auto cata = new QComboBox;
    foreach(auto it, config->Flts)
    {
        cata->addItem(it.second, it.first);
    }
    cata->addItem("Temp", -1);

    auto typeCata = new QComboBox;
    QStringList words;
    typeCata->addItem("-", 0);
    foreach(auto it, config->deckTypes)
    {
        typeCata->addItem(it.second, it.first);
        words << it.second;
    }
    auto completer = new QCompleter(words, this);
    typeCata->setEditable(true);
    typeCata->setCompleter(completer);

    auto switchButton = new QPushButton("...");
    switchButton->setMaximumWidth(20);

    auto refreshOrAbortButton = new IconButton(":/icons/refresh.png", config->getStr("action", "refresh", "刷新"));
    refreshOrAbortButton->setStyleSheet("QPushButton {border: none;} QPushButton:hover {border-width: 1px;border-style: solid; border-radius: 4px; border-color: white}");
    arr.set('a', cata);
    arr.set('b', typeCata);
    arr.set('c', refreshOrAbortButton);
    arr.set('1', switchButton);
    vbox->addLayout(grid);

    decklist = new DeckList;
    vbox->addWidget(decklist);

    auto hbox = new QHBoxLayout;
    hbox->setMargin(2);

    auto buttom = new QWidget;

    pageBox = new QSpinBox();
    pageBox->setMinimum(1);
    pageBox->setKeyboardTracking(false);

    auto left = new IconButton(":/icons/left.png");
    auto right = new IconButton(":/icons/right.png");

    hbox->addWidget(pageBox);
    hbox->addWidget(left);
    hbox->addWidget(right);
    buttom->setLayout(hbox);

    vbox->addWidget(buttom);

    setLayout(vbox);

    connect(pageBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &DeckListView::goPage);

    connect(networking.get(), &NetWorking::list, decklist, &DeckList::setList, Qt::QueuedConnection);
    connect(networking.get(), &NetWorking::ready, this, [=] (bool e) {
        pageBox->setEnabled(e);
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

    connect(decklist, &DeckList::selectDeck, this, &DeckListView::selectDeck);

    connect(refreshOrAbortButton, &IconButton::clicked, [=]()
    {
        if(networking->getWaiting())
        {
            networking->abort();
            refreshOrAbortButton->setIcon(QIcon(":/icons/refresh.png"));
            pageBox->setEnabled(true);
            left->setEnabled(true);
            right->setEnabled(true);
        }
        else
        {
            getList(pageBox->value());
            refreshOrAbortButton->setIcon(QIcon(":/icons/abort.png"));
        }
    });

    connect(cata, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
        config->Flt = cata->itemData(index).toInt();
    });
    connect(typeCata, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
        config->deckType = typeCata->itemData(index).toInt();
    });


    connect(right, &IconButton::clicked, [=]()
    {
        goPage(pageBox->value() + 1);
    });
    connect(left, &IconButton::clicked, [=]()
    {

        goPage(pageBox->value() - 1);
    });

    connect(decklist, &DeckList::deckType, [=] (QString type) {
        config->tempConfig = type;
        config->Flt = -1;
        cata->setCurrentIndex(cata->count() - 1);
        goPage(1);
    });

    connect(switchButton, &QPushButton::clicked, [=] () {
        bool ok;
        QString text = QInputDialog::getText(this, "getlistparam", "", QLineEdit::Normal,
                                             config->tempConfig, &ok);
        if(ok)
        {
            config->tempConfig = text;
            config->Flt = -1;
            cata->setCurrentIndex(cata->count() - 1);
            goPage(1);
        }
    });
}

void DeckListView::setList(Type::DeckL ls)
{
    decklist->setList(ls);
}

void DeckListView::getList(int newPage)
{
    pageBox->setValue(newPage);
    networking->getList(newPage);
}

void DeckListView::goPage(int newPage)
{
    if(newPage > 0)
    {
        getList(newPage);
    }
}
