#include "deckwidget.h"
#include "draghelper.h"
#include "signaltower.h"
#include "limitcards.h"
#include <QDebug>
#include <QApplication>

DeckWidget::DeckWidget(QWidget *parent, int _row, int _column, QSharedPointer<Type::DeckI> _deck)
    : QWidget(parent), m_deck(_deck), m_row(_row), m_column(_column), m_currentCardId(0),
      m_deckSize(-1), m_current(-1), m_overlapV(false)
{
    m_offset = QSize(3, 3);
    m_spacing = QSize(3, 3);
    m_cardSize = QSize(177 / 3.5 , 254 / 3.5);

    m_extFilter = [](quint32) {return true;};

    m_filter = [](quint32 id)
    {
        return (bool)CardManager::inst().getCard(id);
    };

    m_makeSnapShot = []() {};
    QSize min = m_cardSize + m_spacing;
    min.setWidth(min.width() * m_column + m_offset.width() * 2);
    min.setHeight(min.height() * m_row + m_offset.height() * 2);
    min = m_offset + min;
    setMouseTracking(true);
    setMinimumSize(min);

    setAcceptDrops(true);
}

void DeckWidget::paintEvent(QPaintEvent *)
{

    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    if(m_deckSize != m_deck->size())
    {
        emit sizeChanged(m_deck->size());
        m_deckSize = m_deck->size();
    }

    emit deckChanged(*m_deck);

    int cardPerRow = std::max(static_cast<int>(ceil(m_deckSize * 1.0 / m_row)), m_column);

    double cardHeight = (height() - m_offset.height() * 2.0) / m_row - m_spacing.height();
    double cardWidth = (width() - m_offset.width() * 2.0) / cardPerRow - m_spacing.width();
    double timesH = cardHeight / 254;
    double timesW = cardWidth / 177;
    double times = m_overlapV ? std::max(timesH, timesW) : timesH;
    m_cardSize = QSize(177 * times, 254 * times);

    int varWidth = (width() - m_offset.width()) * 1.0 - m_cardSize.width() - m_offset.width();
    int varHeight = (height() - m_offset.height() * 1.0 - m_cardSize.height()) - m_offset.height();
    int yoff = ((height() - m_offset.height()) * 1.0 - (m_cardSize.height() + m_spacing.height()) * m_row) / 2;
    int actualRow = static_cast<int>(ceil(m_deck->size() * 1.0 / cardPerRow));
    bool needOverlap = (actualRow * (m_cardSize.height() + m_spacing.height())) + m_offset.height() * 2 > height();
    auto it = m_deck->begin();
    for(int i = 0; i < m_row && it != m_deck->end(); i++)
    {
        for(int j = 0; j < cardPerRow && it != m_deck->end(); j++)
        {
            int x = m_offset.width() + floor(varWidth * j / (cardPerRow - 1));
            int y = 0;
            if(m_overlapV)
            {
                if(needOverlap)
                {
                    y = m_offset.height() + (actualRow <= 1 ? 0 : floor(varHeight * i) / (actualRow - 1));
                }
                else
                {
                    y = m_offset.height() + i * (m_cardSize.height() + m_spacing.height());
                }
            }
            else
            {
                y = yoff + m_offset.height() + i * (m_cardSize.height() + m_spacing.height());
            }
            it->setPos(QPoint(x, y));
            painter.drawPixmap(x, y, m_cardSize.width(),
                               m_cardSize.height(), *it->getPixmap().data());

            int lim = LimitCards::inst().getLimit(it->getId());
            if(lim < 3)
            {
                auto data = LimitCards::inst().getPixmap(lim);
                if(data)
                {
                    painter.drawPixmap(x, y, 16, 16, *data);
                }
            }
            if(m_current == i * cardPerRow + j)
            {
                QPen pen = painter.pen(), newPen = painter.pen();
                newPen.setWidth(2);
                newPen.setColor(QColor("darkgray"));
                painter.setPen(newPen);
                painter.drawRect(QRect(QPoint(x - 1,  y - 1), m_cardSize + QSize(1, 1)));
                painter.setPen(pen);
            }
            it++;
        }
    }
    painter.drawRect(QRect(QPoint(0, 0), QWidget::size() - QSize(1, 1)));
}

int DeckWidget::itemAt(const QPoint &pos)
{
    int i = m_deck->size();
    for(auto it = m_deck->end(); it != m_deck->begin() && i > 0;)
    {
        --it;
        --i;
        QPoint cardPos = it->getPos();
        if(pos.x() > cardPos.x() && pos.y() > cardPos.y())
        {
            if(pos.x() < cardPos.x() + m_cardSize.width() &&
                    pos.y() < cardPos.y() + m_cardSize.height())
            {
                return i;
            }
        }
    }
    return -1;
}

void DeckWidget::checkLeave()
{

    int i = itemAt(mapFromGlobal(QCursor::pos()));
    if(i != m_current)
    {
        m_current = i;
        update();
    }
}

int DeckWidget::posIndex(const QPoint &pos)
{
    int i = m_deck->size();
    bool found = false;
    for(auto it = m_deck->end(); it != m_deck->begin() && i > 0;)
    {
        --it;
        --i;
        QPoint cardPos = it->getPos();
        if(pos.x() > cardPos.x() && pos.y() > cardPos.y())
        {
            if(pos.y() < cardPos.y() + m_cardSize.height() + m_spacing.height())
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
        int center = m_deck->at(i).getPos().x() + m_cardSize.width() / 2;
        if(pos.x() > center && i < m_deck->size())
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
        m_startPos = event->pos();
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
            quint32 id = m_deck->at(index).getId();
            SignalTower::inst().cardDetails(id);
        }
    }
    QWidget::mousePressEvent(event);
}

void DeckWidget::mouseMoveEvent(QMouseEvent *event)
{
    int index = itemAt(event->pos());
    if(index != -1)
    {
        quint32 id = m_deck->at(index).getId();
        if(m_currentCardId != id)
        {
            m_currentCardId = id;
            SignalTower::inst().changeCurrentId(id);
        }
    }

    if(event->buttons() & Qt::LeftButton)
    {
        int dist = (event->pos() - m_startPos).manhattanLength();
        if(dist >= QApplication::startDragDistance() && index != -1)
        {
            m_current = -1;
            startDrag(index);
        }
    }
    else if(m_current != index)
    {
        m_current = index;
        update();
    }
    QWidget::mouseMoveEvent(event);
}

void DeckWidget::startDrag(int index)
{
    auto *mimedata = new QMimeData;
    CardItem card(m_deck->at(index));
    mimedata->setText(QString::number(card.getId()));
    auto *drag = new QDrag(this);
    drag->setMimeData(mimedata);
    if(card.getPixmap())
    {
        drag->setPixmap(card.getPixmap()->scaled(m_cardSize));
        drag->setHotSpot(QPoint(drag->pixmap().width() / 2,
                                drag->pixmap().height() / 2));
    }
    m_makeSnapShot();
    bool copy = true;
    DragHelper::inst().atomic = true;
    if((QApplication::keyboardModifiers() & Qt::ControlModifier) == 0)
    {
        m_deck->removeAt(index);
        copy = false;
    }
    emit sizeChanged(m_deck->size());
    emit deckChanged(*m_deck);
    update();
    DragHelper::inst().moved = false;
    drag->exec(Qt::MoveAction);
    if(!DragHelper::inst().moved && !copy)
    {
        m_deck->append(card);
    }
    DragHelper::inst().atomic = false;
    update();
}
void DeckWidget::setDeck(const QSharedPointer<Type::DeckI> value)
{
    m_deck = value;
}


void DeckWidget::dragEnterEvent(QDragEnterEvent *event)
{
    auto src = event->source();

    if(src)
    {
        quint32 id = event->mimeData()->text().toUInt();
        if(m_filter(id) && m_extFilter(id))
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
        quint32 id = event->mimeData()->text().toUInt();
        if(m_filter(id) && m_extFilter(id))
        {
            if(!DragHelper::inst().atomic)
            {
                m_makeSnapShot();
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
            DragHelper::inst().moved = true;
            update();
        }
        event->accept();
    }
}

void DeckWidget::deleteCard(QPoint _pos)
{
    int index = itemAt(_pos);
    m_makeSnapShot();
    m_deck->removeAt(index);
    update();
}

int DeckWidget::countCard(quint32 id)
{
    if(auto ocard = CardManager::inst().getCard(id))
    {
        Card &card = **ocard;

        int sum = 0;
        foreach(auto &item, *m_deck)
        {
            if(auto ocard = CardManager::inst().getCard(item.getId()))
            {
                Card &card2 = **ocard;
                if(id == card2.id)
                {
                    sum += 1;
                }
                else if(id == card2.alias)
                {
                    sum += 1;
                }
                else if(card.alias == card2.id)
                {
                    sum += 1;
                }
                else if(card.alias == card2.alias && card.alias != 0)
                {
                    sum += 1;
                }
            }
        }
        return sum;
    }
    else
    {
        return 0;
    }
}

void DeckWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        if(itemAt(mapFromGlobal(QCursor::pos())) >= 0)
        {
            SignalTower::inst().IdClick(m_currentCardId);
        }
    }
    QWidget::mouseDoubleClickEvent(event);
}

DeckWidget::~DeckWidget()
{

}

