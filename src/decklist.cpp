#include "decklist.h"
#include "iconbutton.h"
#include "config.h"
#include <QDebug>
#include <QComboBox>

DeckList::DeckList(QWidget *parent)
    : QListWidget(parent)
{
    connect(this, &DeckList::itemClicked, this, &DeckList::onItem);
    connect(this, &DeckList::itemDoubleClicked, this, &DeckList::openURL);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

DeckList::~DeckList()
{

}

void DeckList::setList(QSharedPointer<QList<QPair<QString, QString> > > ls)
{
    clear();
    foreach(auto &it, *ls)
    {
        auto item = new QListWidgetItem;
        item->setText(it.first);
        item->setData(Qt::UserRole, it.second);
        addItem(item);
    }
}

void DeckList::onItemChanged()
{
    onItem(currentItem());
}

void DeckList::onItem(QListWidgetItem *item)
{
    QString id = item->data(Qt::UserRole).toString();
    emit selectDeck(id, item->text());
}

void DeckList::openURL(QListWidgetItem* item)
{
    QString id = item->data(Qt::UserRole).toString();
    QUrl url(config->getCurrentRemote().openurl.replace("~0", id));
    QDesktopServices::openUrl(url);
}

DeckListView::DeckListView(QWidget *parent)
    : QWidget(parent), page(1)
{
    auto vbox = new QVBoxLayout;
    decklist = new DeckList;

    vbox->addWidget(decklist);

    auto cata = new QComboBox;
    int index = 0;
    foreach(auto &remoteConfig, config->remoteConfigs)
    {
        cata->addItem(remoteConfig.str, index++);
    }
    if(cata->count() > 0) {
        cata->setCurrentIndex(0);
    }
    connect(cata, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            config, &Config::setRemote);
    vbox->addWidget(cata);

    auto buttom = new QWidget;
    auto hbox = new QHBoxLayout;

    pageEdit = new QLineEdit(tr("1"));
    //pageEdit->setMinimumWidth(50);
    auto goButton = new IconButton(":/icons/goto.png", config->getStr("action", "goto", "跳转"));
    auto nextButton = new IconButton(":/icons/right.png", config->getStr("action", "next", "下一页"));
    auto prevButton = new IconButton(":/icons/left.png", config->getStr("action", "prev", "上一页"));
    auto abortButton = new IconButton(":/icons/abort.png", config->getStr("action", "abort", "中止"));
    abortButton->setEnabled(false);

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

    connect(decklist, &DeckList::selectDeck, &remote, &Remote::getDeck);
    connect(&remote, &Remote::deckStream, this, &DeckListView::deckStream);

    connect(&remote, &Remote::list, decklist, &DeckList::setList);
    connect(&remote, &Remote::ready, abortButton, &IconButton::setDisabled);
    connect(abortButton, &IconButton::clicked, &remote, &Remote::abort);
}

void DeckListView::setList(QSharedPointer<QList<QPair<QString, QString> > > ls)
{
    decklist->setList(ls);
}

void DeckListView::nextPage()
{
    if(!remote.is_waiting())
    {
        page++;
        pageEdit->setText(QString::number(page));
        getList();
    }
}

void DeckListView::prevPage()
{
    if(!remote.is_waiting() && page > 1)
    {
        page--;
        pageEdit->setText(QString::number(page));
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
