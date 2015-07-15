#include "decklist.h"
#include "iconbutton.h"
#include "config.h"
#include <QDebug>
#include <QComboBox>

DeckList::DeckList(QWidget *parent)
    : QListWidget(parent), menuItem(nullptr)
{
    popup = new QMenu(this);
    auto sameAction = new QAction(popup);
    sameAction->setText(config->getStr("action", "same", "同类卡组"));
    popup->addAction(sameAction);

    connect(sameAction, &QAction::triggered, this, &DeckList::same);
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
    emit selectDeck(id, item->text());
}

void DeckList::openURL(QListWidgetItem* item)
{
    QString id = item->data(Qt::UserRole).toList()[0].toString();
    QUrl url(config->getCurrentRemote().openurl.replace("~0", id));
    QDesktopServices::openUrl(url);
}

DeckListView::DeckListView(QWidget *parent)
    : QWidget(parent), page(1), lastConfig(0), lastPage(1)
{
    auto vbox = new QVBoxLayout;
    auto hbox = new QHBoxLayout;
    decklist = new DeckList;

    vbox->addWidget(decklist);

    auto cata = new QComboBox;
    int index = 0;
    foreach(auto &remoteConfig, config->remoteConfigs)
    {
        cata->addItem(remoteConfig.str, index++);
    }
    cata->addItem(tempRemoteConfig.str, -1);
    if(cata->count() > 0) {
        cata->setCurrentIndex(0);
    }

    auto switchButton = new QPushButton("...");
    auto backButton = new IconButton(":/icons/back.png", config->getStr("action", "back", "返回"));
    hbox = new QHBoxLayout;
    hbox->addWidget(cata);
    hbox->addWidget(switchButton);
    hbox->addWidget(backButton);
    vbox->addLayout(hbox);

    auto buttom = new QWidget;

    pageEdit = new QLineEdit(tr("1"));
    auto goButton = new IconButton(":/icons/goto.png", config->getStr("action", "goto", "跳转"));
    auto nextButton = new IconButton(":/icons/right.png", config->getStr("action", "next", "下一页"));
    auto prevButton = new IconButton(":/icons/left.png", config->getStr("action", "prev", "上一页"));
    auto abortButton = new IconButton(":/icons/abort.png", config->getStr("action", "abort", "中止"));
    abortButton->setEnabled(false);

    hbox = new QHBoxLayout;
    hbox->addWidget(pageEdit, 1);
    hbox->addWidget(goButton);
    hbox->addWidget(prevButton);
    hbox->addWidget(nextButton);

    buttom->setLayout(hbox);

    vbox->addWidget(buttom);
    auto refreshButton = new IconButton(":/icons/refresh.png", config->getStr("action", "refresh", "刷新"));
    connect(refreshButton, &IconButton::clicked, this, &DeckListView::getList);

    hbox = new QHBoxLayout;
    hbox->addWidget(refreshButton);
    hbox->addWidget(abortButton);
    vbox->addLayout(hbox);

    setLayout(vbox);

    connect(nextButton, &IconButton::clicked, this, &DeckListView::nextPage);
    connect(prevButton, &IconButton::clicked, this, &DeckListView::prevPage);
    connect(goButton, &IconButton::clicked, this, &DeckListView::goPage);

    connect(&remote, &Remote::list, decklist, &DeckList::setList);
    connect(&remote, &Remote::ready, abortButton, &IconButton::setDisabled);
    connect(decklist, &DeckList::selectDeck, this, &DeckListView::selectDeck);
    connect(abortButton, &IconButton::clicked, &remote, &Remote::abort);


    connect(cata, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) {
        config->setRemote(cata->itemData(index).toInt());
    });

    connect(decklist, &DeckList::deckType, [=] (QString type) {
        if(config->remote != -1)
        {
            lastConfig = config->remote;
            lastPage = page;
        }
        tempRemoteConfig = config->getCurrentRemote();
        tempRemoteConfig.getlistparam = type;
        page = 1;
        cata->setCurrentIndex(cata->count() - 1);
        config->setRemote(-1);
        getList();
    });

    connect(switchButton, &QPushButton::clicked, [=] () {
        bool ok;
        QString text = QInputDialog::getText(this, "getlistparam", "", QLineEdit::Normal,
                                             config->getCurrentRemote().getlistparam, &ok);
        if(ok)
        {
            if(config->remote != -1)
            {
                lastConfig = config->remote;
                lastPage = page;
            }
            tempRemoteConfig = config->getCurrentRemote();
            tempRemoteConfig.getlistparam = text;
            page = 1;
            cata->setCurrentIndex(cata->count() - 1);
            config->setRemote(-1);
            getList();
        }
    });
    connect(backButton, &IconButton::clicked, [=] () {
        config->setRemote(lastConfig);
        cata->setCurrentIndex(lastConfig);
        page = lastPage;
        pageEdit->setText(QString::number(page));
        getList();
    });
}

void DeckListView::setList(Type::DeckL ls)
{
    decklist->setList(ls);
}

void DeckListView::nextPage()
{
    if(!remote.is_waiting())
    {
        page++;
        getList();
    }
}

void DeckListView::prevPage()
{
    if(!remote.is_waiting() && page > 1)
    {
        page--;
        getList();
    }
}

void DeckListView::goPage()
{
    int newPage = pageEdit->text().toInt();
    if(newPage > 0)
    {
        page = newPage;
        getList();
    }
}
