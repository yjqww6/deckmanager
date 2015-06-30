#include "deckwidget.h"
#include "draghelper.h"
#include <ctime>
#include <random>
#include <QDebug>

DeckWidget::DeckWidget(QWidget *parent, int _row, int _column)
    : QWidget(parent), row(_row), column(_column), currentCardId(0),
      deckSize(-1), current(-1)
{
    offset = QSize(2, 2);
    spacing = QSize(3, 3);
    cardSize = QSize(177 / 3.5 , 254 / 3.5);

    extFilter = [](int) {return true;};
    makeSnapShot = []() {};
    QSize min = cardSize + spacing;
    min.setWidth(min.width() * column + offset.width() * 2);
    min.setHeight(min.height() * row + offset.height() * 2);
    min = offset + min - spacing;
    setMouseTracking(true);
    setMinimumSize(min);

    setAcceptDrops(true);
}

void DeckWidget::resizeEvent(QResizeEvent *event)
{
    double cardHeight = (height() - offset.height() * 2.0) / row - spacing.height();
    double cardWidth = (width() - offset.width() * 2.0) / column - spacing.width();
    double timesH = cardHeight / 254, timesW = cardWidth / 177;
    double times = std::min(timesH, timesW);
    cardSize = QSize(177 * times, 254 * times);
    QWidget::resizeEvent(event);
//    int newHeight = (cardSize.height() + spacing.height()) * row + offset.height() * 2;
//    if(height() != newHeight)
//    {
//        resize(width(), newHeight);
//    }
}

void DeckWidget::paintEvent(QPaintEvent *)
{

    QPainter painter(this);
    if(deckSize != deck.size())
    {
        emit sizeChanged(deck.size());
        deckSize = deck.size();
    }

    emit deckChanged(deck);
    int cardPerRow = std::max(static_cast<int>(ceil(deckSize * 1.0 / row)), column);
    int varWidth = (width() - offset.width()) * 1.0 - cardSize.width() - offset.width();
    auto it = deck.begin();
    for(int i = 0; i < row && it != deck.end(); i++)
    {
        for(int j = 0; j < cardPerRow && it != deck.end(); j++)
        {
            int x = offset.width() + floor(varWidth * j / (cardPerRow - 1)),
                    y = offset.height() + i * (cardSize.height() + spacing.height());
            it->setPos(QPoint(x, y));
            painter.drawPixmap(x, y, cardSize.width(),
                               cardSize.height(), *it->getPixmap().data());

            int lim = LimitCards::getLimit(it->getId());
            if(lim < 3)
            {
                auto data = LimitCards::getPixmap(lim);
                if(data)
                {
                    painter.drawPixmap(x, y, 16, 16, *data.data());
                }
            }
            if(current == i * cardPerRow + j)
            {
                QPen pen = painter.pen(), newPen = painter.pen();
                newPen.setWidth(2);
                newPen.setColor(QColor("darkgray"));
                painter.setPen(newPen);
                painter.drawRect(QRect(QPoint(x - 1,  y - 1), cardSize + QSize(1, 1)));
                painter.setPen(pen);
            }
            it++;
        }
    }
    painter.drawRect(QRect(QPoint(0, 0), QWidget::size() - QSize(1, 1)));
}

int DeckWidget::itemAt(const QPoint &pos)
{
    int i = deck.size();
    for(auto it = deck.end(); it != deck.begin() && i > 0;)
    {
        --it;
        --i;
        QPoint cardPos = it->getPos();
        if(pos.x() > cardPos.x() && pos.y() > cardPos.y())
        {
            if(pos.x() < cardPos.x() + cardSize.width() &&
                    pos.y() < cardPos.y() + cardSize.height())
            {
                return i;
            }
        }
    }
    return -1;
}

void DeckWidget::shuffle()
{
    makeSnapShot();
    std::random_shuffle(deck.begin(), deck.end());
    update();
}

void DeckWidget::sort()
{
    qSort(deck.begin(), deck.end(),[&](CardItem &a, CardItem &b)
    {
        auto ca = CardPool::getCard(a.getId());
        auto cb = CardPool::getCard(b.getId());
        int ta = ca->type & 7, tb = cb->type & 7;
        if(ta != tb)
        {
            return ta < tb;
        }
        else if(ca->type != cb->type)
        {
            return ca->type < cb->type;
        }
        else if(ca->type & Card::TYPE_MONSTER)
        {
            if(ca->level != cb->level)
            {
                return ca->level >= cb->level;
            }

            if(ca->atk != cb->atk)
            {
                return ca->atk >= cb->atk;
            }
            if(ca->def != cb->def)
            {
                return ca->def >= cb->def;
            }
            return ca->id < cb->id;
        }
        else
        {
            return ca->id < cb->id;
        }
    });
    update();
}

void DeckWidget::checkLeave()
{

    int i = itemAt(mapFromGlobal(QCursor::pos()));
    if(i != current)
    {
        current = i;
        update();
    }
}

int DeckWidget::posIndex(const QPoint &pos)
{
    int i = deck.size();
    bool found = false;
    for(auto it = deck.end(); it != deck.begin() && i > 0;)
    {
        --it;
        --i;
        QPoint cardPos = it->getPos();
        if(pos.x() > cardPos.x() && pos.y() > cardPos.y())
        {
            if(pos.y() < cardPos.y() + cardSize.height() + spacing.height())
            {
                found = true;
                break;
            }
        }
    }

    if(!found)
    {
        return -1;
    }
    else
    {
        int center = deck[i].getPos().x() + cardSize.width() / 2;
        if(pos.x() > center && i < deck.size())
        {
            return i + 1;
        }
        else
        {
            return i;
        }
    }
}

void DeckWidget::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        startPos = event->pos();
    }
    else if(event->buttons() & Qt::RightButton)
    {
        deleteCard(event->pos());
    }
    else if(event->buttons() & Qt::MiddleButton)
    {
        int index = itemAt(event->pos());
        if(index >= 0)
        {
            int id = deck[index].getId();
            emit details(id);
        }
    }
    QWidget::mousePressEvent(event);
}

void DeckWidget::mouseMoveEvent(QMouseEvent *event)
{
    int index = itemAt(event->pos());
    if(index != -1)
    {
        int id = deck[index].getId();
        if(currentCardId != id)
        {
            currentCardId = id;
            emit currentIdChanged(id);
        }
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

void DeckWidget::startDrag(int index)
{
    auto *mimedata = new QMimeData;
    CardItem card(deck[index]);
    mimedata->setText(QString::number(card.getId()));
    auto *drag = new QDrag(this);
    drag->setMimeData(mimedata);
    drag->setPixmap(card.getPixmap()->scaled(cardSize));
    drag->setHotSpot(QPoint(drag->pixmap().width() / 2,
                            drag->pixmap().height() / 2));
    makeSnapShot();
    bool copy = true;
    dragHelper.atomic = true;
    if((QApplication::keyboardModifiers() & Qt::ControlModifier) == 0)
    {
        deck.removeAt(index);
        copy = false;
    }
    emit sizeChanged(deck.size());
    emit deckChanged(deck);
    update();
    dragHelper.moved = false;
    drag->exec(Qt::MoveAction);
    if(!dragHelper.moved && !copy)
    {
        deck.append(card);
    }
    dragHelper.atomic = false;
    update();
}

void DeckWidget::dragEnterEvent(QDragEnterEvent *event)
{
    auto src = event->source();

    if(src)
    {
        int id = event->mimeData()->text().toInt();
        if(filter(id) && extFilter(id))
        {
          event->accept();
        }
        else
        {
            event->ignore();
        }
    }
}

void DeckWidget::dragMoveEvent(QDragMoveEvent *event)
{
    auto src = event->source();
    if(src)
    {
        event->accept();
    }
}

void DeckWidget::dropEvent(QDropEvent *event)
{
    auto src = event->source();
    if(src)
    {
        int id = event->mimeData()->text().toInt();
        if(filter(id) && extFilter(id))
        {
            if(!dragHelper.atomic)
            {
                makeSnapShot();
            }
            int index = posIndex(event->pos());
            if(index == -1)
            {
                addCard(id);
            }
            else
            {
                insertCard(index, id);
            }
            dragHelper.moved = true;
            update();
        }
        event->accept();
    }
}

void DeckWidget::deleteCard(QPoint _pos)
{
    int index = itemAt(_pos);
    makeSnapShot();
    deck.removeAt(index);
    update();
}

int DeckWidget::countCard(int id)
{
    auto card = CardPool::getCard(id);
    int sum = 0;
    foreach(const CardItem &item, deck)
    {
        auto card2 = CardPool::getCard(item.getId());
        if(static_cast<quint32>(id) == card2->id)
        {
            sum += 1;
        }
        else if(static_cast<quint32>(id) == card2->alias)
        {
            sum += 1;
        }
        else if(card->alias == card2->id)
        {
            sum += 1;
        }
        else if(card->alias == card2->alias && card->alias != 0)
        {
            sum += 1;
        }
    }
    return sum;
}

DeckWidget::~DeckWidget()
{

}

