#include "decklist.h"
#include "iconbutton.h"
#include "config.h"
#include <QDebug>

DeckList::DeckList(QWidget *parent)
    : QListWidget(parent)
{
    connect(this, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(onItem(QListWidgetItem*)));
    connect(this, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            this, SLOT(openURL(QListWidgetItem*)));
}

DeckList::~DeckList()
{

}

void DeckList::setList(QSharedPointer<QList<QPair<QString, QString> > > ls)
{
    clear();
    for(auto it = ls->begin(); it != ls->end(); ++it)
    {
        auto item = new QListWidgetItem;
        item->setText(it->first);
        item->setData(Qt::UserRole, it->second);
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
    QUrl url(config->getStr("remote", "openurl", "").replace("~0", id));
    QDesktopServices::openUrl(url);
}

DeckListView::DeckListView(QWidget *parent)
    : QWidget(parent), page(1)
{
    auto vbox = new QVBoxLayout;
    decklist = new DeckList;

    vbox->addWidget(decklist);

    auto buttom = new QWidget;
    auto hbox = new QHBoxLayout;

    pageEdit = new QLineEdit(tr("1"));
    pageEdit->setMinimumWidth(50);
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
    connect(refreshButton, SIGNAL(clicked()), this, SLOT(getList()));

    hbox = new QHBoxLayout;
    hbox->addWidget(refreshButton);
    hbox->addWidget(abortButton);
    vbox->addLayout(hbox);

    setLayout(vbox);

    connect(nextButton, SIGNAL(clicked()), this, SLOT(nextPage()));
    connect(prevButton, SIGNAL(clicked()), this, SLOT(prevPage()));
    connect(goButton, SIGNAL(clicked()), this, SLOT(goPage()));

    connect(decklist, SIGNAL(selectDeck(QString, QString)), &remote, SLOT(getDeck(QString, QString)));
    connect(&remote, SIGNAL(deckStream(QString, QString, bool)),
            this, SIGNAL(deckStream(QString,QString,bool)));

    connect(&remote, SIGNAL(list(QSharedPointer<QList<QPair<QString, QString> > >)),
            decklist, SLOT(setList(QSharedPointer<QList<QPair<QString, QString> > >)));
    connect(&remote, SIGNAL(ready(bool)), abortButton, SLOT(setDisabled(bool)));
    connect(abortButton, SIGNAL(clicked()), &remote, SLOT(abort()));
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
