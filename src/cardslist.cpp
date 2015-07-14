#include "cardslist.h"
#include "limitcards.h"
#include "draghelper.h"
#include "range.h"
#include <QToolBar>
#include <QDebug>

CardsList::CardsList(QWidget *parent)
    : QWidget(parent), pos(0), cardSize(177 / 3.5, 254 / 3.5), cardsPerColumn(0),
      sb(nullptr), needRefreshId(false), current(-1)
{
    setMouseTracking(true);
    setAcceptDrops(true);
    setMinimumWidth(cardSize.width() + fontMetrics().width(tr("宽")) * 10);
    auto family = font().family();
    setFont(QFont(family, 11));
}

void CardsList::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;

    needRefreshId = true;
    point = event->pos();

    setPos(pos - numSteps);
    event->accept();
}

void CardsList::refresh()
{
    int max = ls.size() - cardsPerColumn;
    if(sb)
    {
        sb->setMaximum(max > 0 ? max : 0);
    }
}

void CardsList::setScrollBar(QScrollBar *_sb)
{
    sb = _sb;
    sb->setMaximum(ls.size());
    connect(sb, &QScrollBar::valueChanged, this, &CardsList::setPos);
}

QString CardsList::adToString(int ad)
{
    if(ad == -2)
    {
        return "?";
    }
    else
    {
        return QString::number(ad);
    }
}

void CardsList::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if(!ls.empty())
    {
        int fmHeight = painter.fontMetrics().height();
        int h = height(), cardHeight = cardSize.height(),
                cardWidth = cardSize.width();

        cardsPerColumn = h / cardHeight;

        decltype(items) newItems;

        double varHeight = h - cardHeight * 1.0;

        for(int i : range(std::min(cardsPerColumn, ls.size() - pos)))
        {
            quint32 id = ls[i + pos];

            auto it = newItems.insert(i + pos, CardItem(id));

            auto &item = it.value();

            int y = varHeight * i / (cardsPerColumn - 1);


            current = itemAt(mapFromGlobal(QCursor::pos()));
            if(i + pos == current)
            {
                QBrush brush = painter.brush(), newBrush(Qt::lightGray);
                QPen pen = painter.pen();
                painter.setPen(Qt::transparent);
                QColor color(newBrush.color());
                color.setAlpha(160);
                newBrush.setColor(color);
                painter.setBrush(newBrush);
                painter.drawRect(QRect(QPoint(0,  y), QSize(sb->geometry().x(), cardSize.height())));
                painter.setBrush(brush);
                painter.setPen(pen);
            }

            item.setPos(QPoint(0, y));

            painter.drawPixmap(0, y, cardWidth, cardHeight,
                               *item.getPixmap().data());

            int lim = limitCards->getLimit(it->getId());
            if(lim < 3)
            {
                auto data = limitCards->getPixmap(lim);
                if(data)
                {
                    painter.drawPixmap(0, y, 16, 16, *data);
                }
            }

            call_with_ref([&](Card &card) {

                painter.drawText(cardWidth + 5, y + fmHeight, card.name);
                QString ot;
                QString level = (card.type & Const::TYPE_XYZ) ? tr("R") : tr("L");
                level = "[" + level + QString::number(card.level) + "]";
                if((card.ot & 0x3) == 1)
                {
                    ot = tr("[OCG]");
                }
                else if((card.ot & 0x3) == 2)
                {
                    ot = tr("[TCG]");
                }

                if(card.type & Const::TYPE_MONSTER)
                {
                    painter.drawText(cardWidth + 5, y + 5 + fmHeight * 2,
                                     card.cardRace() + tr("/") + card.cardAttr() + level);

                    painter.drawText(cardWidth + 5, y + 10 + fmHeight * 3,
                                     adToString(card.atk) + tr("/") +
                                     adToString(card.def) + ot);
                }
                else if(card.type & (Const::TYPE_SPELL | Const::TYPE_TRAP))
                {
                    painter.drawText(cardWidth + 5, y + 5 + fmHeight * 2,
                                     card.cardType());
                    painter.drawText(cardWidth + 5, y + 10 + fmHeight * 3, ot);
                }
            }, cardPool->getCard(id));
        }
        items.swap(newItems);
    }
    if(needRefreshId)
    {
        refreshCurrentId();
        needRefreshId = false;
    }
    refresh();
}

void CardsList::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        startPos = event->pos();
    }
    else if(event->buttons() & Qt::MiddleButton)
    {
        int index = itemAt(event->pos());
        if(index >= 0)
        {
            emit details(ls[index]);
        }
    }
    QWidget::mousePressEvent(event);
}

int CardsList::itemAt(const QPoint &_pos)
{
    for(int i: range(cardsPerColumn))
    {
        if(i + pos >= ls.size())
        {
            break;
        }
        int index = i +  pos;

        auto &item = items.find(index).value();

        if(_pos.y() >= item.getPos().y() &&
                _pos.y() <= item.getPos().y() + cardSize.height() &&
                _pos.x() >= 0 && _pos.x() < width())
        {
            return index;
        }
    }
    return -1;
}

void CardsList::refreshCurrentId()
{
    int index = itemAt(point);
    if(index != -1)
    {
        quint32 id = ls[index];
        if(currentCardId != id)
        {
            currentCardId = id;
            emit currentIdChanged(ls[index]);
        }
    }
}

void CardsList::mouseMoveEvent(QMouseEvent *event)
{
    int index = itemAt(event->pos());
    if(index != -1)
    {
        quint32 id = ls[index];
        if(currentCardId != id)
        {
            currentCardId = id;
            emit currentIdChanged(ls[index]);
        }
        needRefreshId = false;
    }

    if(event->buttons() & Qt::LeftButton)
    {
        int dist = (event->pos() - startPos).manhattanLength();
        if(dist >= QApplication::startDragDistance() && index != -1)
        {
            current = -1;
            startDrag(index);
        }
    }
    else if(current != index)
    {
        current = index;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void CardsList::startDrag(int index)
{
    if(index >= ls.size())
    {
        return;
    }
    QMimeData *mimedata = new QMimeData;
    quint32 id = ls[index];
    mimedata->setText(QString::number(id));
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimedata);
    auto &item = items.find(index).value();
    if(item.getPixmap())
    {
        drag->setPixmap(item.getPixmap()->scaled(cardSize));
        drag->setHotSpot(QPoint(drag->pixmap().width() / 2, drag->pixmap().height() / 2));
    }
    dragHelper.moved = false;
    drag->exec(Qt::MoveAction);
}

void CardsList::dragEnterEvent(QDragEnterEvent *event)
{
    QObject *src = event->source();

    if(src)
    {
        event->accept();
    }
}

void CardsList::dragMoveEvent(QDragMoveEvent *event)
{
    QObject *src = event->source();
    if(src)
    {
        event->accept();
    }
}

void CardsList::dropEvent(QDropEvent *event)
{
    dragHelper.moved = true;
    event->accept();
}

void CardsList::setPos(int _pos)
{
    int max = ls.size() - cardsPerColumn;
    max = max > 0 ? max : 0;
    if(pos > max)
    {
        pos = max;
    }
    if(_pos >= 0 && _pos <= max)
    {
        pos = _pos;
        update();
    }
    if(sb)
    {
        sb->setValue(pos);
    }
}

void CardsList::setCards(Type::DeckP cards)
{
    ls.swap(*cards.data());
    ls.squeeze();

    setPos(0);

    emit sizeChanged(ls.size());
    refresh();
}

void CardsList::checkLeave()
{
    int i = itemAt(mapFromGlobal(QCursor::pos()));
    if(i != current)
    {
        current = i;
        update();
    }
}

void CardsList::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        if(itemAt(mapFromGlobal(QCursor::pos())) >= 0)
        {
            emit clickId(currentCardId);
        }
    }
    QWidget::mouseDoubleClickEvent(event);
}

CardsList::~CardsList()
{

}

CardsListView::CardsListView(QWidget *parent)
    : QWidget(parent)
{
    auto hbox = new QHBoxLayout;
    auto vbox = new QVBoxLayout;

    cl = new CardsList(nullptr);
    auto sb = new QScrollBar;

    auto toolbar = new QToolBar;
    undoAction = new QAction(this);
    undoAction->setToolTip(config->getStr("action", "undo", ""));
    undoAction->setIcon(QIcon(":/icons/left.png"));
    redoAction = new QAction(this);
    redoAction->setToolTip(config->getStr("action", "redo", ""));
    redoAction->setIcon(QIcon(":/icons/right.png"));

    toolbar->addAction(undoAction);
    toolbar->addAction(redoAction);

    auto label = new DeckSizeLabel(config->getStr("label", "number", "数目"));
    label->setAlignment(Qt::AlignVCenter |  Qt::AlignRight);

    toolbar->addSeparator();
    auto spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolbar->addWidget(spacer);
    toolbar->addWidget(label);

    cl->setScrollBar(sb);

    connect(cl, &CardsList::currentIdChanged, this, &CardsListView::changeId);
    connect(cl, &CardsList::sizeChanged, label, &DeckSizeLabel::changeSize);
    connect(cl, &CardsList::clickId, this, &CardsListView::clickId);
    connect(cl, &CardsList::details, this, &CardsListView::details);
    connect(undoAction, &QAction::triggered, this, &CardsListView::undo);
    connect(redoAction, &QAction::triggered, this, &CardsListView::redo);

    toolbar->setStyleSheet("color: black; font-size: 12px");
    if(config->bg)
    {
        cl->setStyleSheet("color: white; font-size: 15px");
        label->setStyleSheet("color: white; font-size: 15px");
    }
    else
    {
        cl->setStyleSheet("font-size: 15px");
        label->setStyleSheet("color: white; font-size: 15px");
    }

    hbox->addWidget(cl);
    hbox->addWidget(sb);
    vbox->addWidget(toolbar);
    vbox->addLayout(hbox, 1);
    setLayout(vbox);
    label->changeSize(0);
    updateButtons();
}

void CardsListView::makeSnapShot()
{
    redoSnapShots.clear();
    if(undoSnapShots.size() >= 5)
    {
        undoSnapShots.pop_front();
    }
    undoSnapShots.append(cl->getList());
}

void CardsListView::updateButtons()
{
    undoAction->setEnabled(undoSnapShots.size() > 0);
    redoAction->setEnabled(redoSnapShots.size() > 0);
}

void CardsListView::setCards(Type::DeckP cards)
{
    makeSnapShot();
    cl->setCards(cards);
    updateButtons();
}

void CardsListView::undo()
{
    if(undoSnapShots.size() == 0)
    {
        return;
    }
    Type::Deck temp;
    temp.swap(cl->getList());
    redoSnapShots.append(std::move(temp));

    auto ptr = Type::DeckP::create();
    ptr->swap(undoSnapShots.back());
    undoSnapShots.pop_back();

    cl->setCards(ptr);
    updateButtons();
}

void CardsListView::redo()
{
    if(redoSnapShots.size() == 0)
    {
        return;
    }
    Type::Deck temp;
    temp.swap(cl->getList());
    undoSnapShots.append(std::move(temp));

    auto ptr = Type::DeckP::create();
    ptr->swap(redoSnapShots.back());
    redoSnapShots.pop_back();

    cl->setCards(ptr);
    updateButtons();
}
