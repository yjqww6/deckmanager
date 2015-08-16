#include "decklist.h"
#include "iconbutton.h"
#include "config.h"
#include <QDebug>
#include <QComboBox>
#include <QCompleter>

DeckList::DeckList(QWidget *parent)
    : QListWidget(parent), menuItem(nullptr)
{
    popup = new QMenu(this);
    auto sameAction = new QAction(popup);
    auto newTabAction = new QAction(popup);
    sameAction->setText(config->getStr("action", "same", "同类卡组"));
    newTabAction->setText(config->getStr("action", "newtab", "在新标签页打开"));
    popup->addAction(sameAction);
    popup->addAction(newTabAction);

    connect(sameAction, &QAction::triggered, this, &DeckList::same);
    connect(newTabAction, &QAction::triggered, this, &DeckList::newTab);
    connect(this, &DeckList::itemClicked, this, &DeckList::onItem);
    connect(this, &DeckList::itemDoubleClicked, this, &DeckList::openURL);
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

void DeckList::openURL(QListWidgetItem* item)
{
    QString id = item->data(Qt::UserRole).toList()[0].toString();
    QString openurl = config->getCurrentRemote().openurl;
    QUrl url(openurl.replace("~0", id));
    QDesktopServices::openUrl(url);
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
    : QWidget(parent), page(1), lastConfig(0), lastPage(1)
{
    auto vbox = new QVBoxLayout;
    auto hbox = new QHBoxLayout;
    hbox->setMargin(2);
    decklist = new DeckList;
    vbox->addWidget(decklist);

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

    hbox = new QHBoxLayout;
    hbox->setMargin(2);
    hbox->addWidget(cata);
    hbox->addWidget(typeCata);
    vbox->addLayout(hbox);

    auto buttom = new QWidget;


    auto switchButton = new QPushButton("...");
    pageBox = new QSpinBox();
    pageBox->setMinimum(1);
    pageBox->setKeyboardTracking(false);

    hbox = new QHBoxLayout;
    hbox->setMargin(2);
    hbox->addWidget(pageBox);
    hbox->addWidget(switchButton);
    buttom->setLayout(hbox);

    vbox->addWidget(buttom);

    auto abortButton = new IconButton(":/icons/abort.png", config->getStr("action", "abort", "中止"));
    auto refreshButton = new IconButton(":/icons/refresh.png", config->getStr("action", "refresh", "刷新"));
    connect(refreshButton, &IconButton::clicked, this, &DeckListView::getList);

    abortButton->setEnabled(false);
    hbox = new QHBoxLayout;
    hbox->setMargin(2);
    hbox->addWidget(refreshButton);
    hbox->addWidget(abortButton);
    vbox->addLayout(hbox);

    setLayout(vbox);

    connect(pageBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &DeckListView::goPage);

    connect(&remote, &Remote::list, decklist, &DeckList::setList);
    connect(&remote, &Remote::ready, [=] (bool e) {
        abortButton->setEnabled(!e);
        pageBox->setEnabled(e);
    });
    connect(decklist, &DeckList::selectDeck, this, &DeckListView::selectDeck);
    connect(abortButton, &IconButton::clicked,
            [=] () {
        remote.abort();
        pageBox->setEnabled(true);
    });


    connect(cata, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
        config->Flt = cata->itemData(index).toInt();
    });
    connect(typeCata, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
        config->deckType = typeCata->itemData(index).toInt();
    });

    connect(decklist, &DeckList::deckType, [=] (QString type) {
        config->tempConfig = type;
        config->Flt = -1;
        page = 1;
        cata->setCurrentIndex(cata->count() - 1);
        getList();
    });

    connect(switchButton, &QPushButton::clicked, [=] () {
        bool ok;
        QString text = QInputDialog::getText(this, "getlistparam", "", QLineEdit::Normal,
                                             config->tempConfig, &ok);
        if(ok)
        {
            config->tempConfig = text;
            page = 1;
            config->Flt = -1;
            cata->setCurrentIndex(cata->count() - 1);
            getList();
        }
    });
}

void DeckListView::setList(Type::DeckL ls)
{
    decklist->setList(ls);
}

void DeckListView::goPage(int newPage)
{
    if(newPage > 0)
    {
        page = newPage;
        getList();
    }
}
