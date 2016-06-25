#include "cardslist.h"
#include "limitcards.h"
#include "draghelper.h"
#include "range.h"
#include "signaltower.h"
#include <QToolBar>
#include <QDebug>

CardsList::CardsList(QWidget *parent)
    : QWidget(parent), m_currentPos(0), m_cardSize(177 / 3.5, 254 / 3.5), m_cardsPerColumn(0),
      m_sb(nullptr), m_needRefreshId(false), m_current(-1)
{
    setMouseTracking(true);
    setAcceptDrops(true);
    setMinimumWidth(m_cardSize.width() + fontMetrics().width("宽") * 10);
    auto family = font().family();
    setFont(QFont(family, 11));
}

void CardsList::wheelEvent(QWheelEvent *event)
{
    int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;

    m_needRefreshId = true;
    m_point = event->pos();

    setPos(m_currentPos - numSteps);
    event->accept();
}

void CardsList::refresh()
{
    int max = m_deck.size() - m_cardsPerColumn;
    if(m_sb)
    {
        m_sb->setMaximum(max > 0 ? max : 0);
    }
}

void CardsList::setScrollBar(QScrollBar *_sb)
{
    m_sb = _sb;
    m_sb->setMaximum(m_deck.size());
    connect(m_sb, &QScrollBar::valueChanged, this, &CardsList::setPos);
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
    if(!m_deck.empty())
    {
        int fmHeight = painter.fontMetrics().height();
        int h = height(), cardHeight = m_cardSize.height(),
                cardWidth = m_cardSize.width();

        m_cardsPerColumn = h / cardHeight;

        decltype(m_items) newItems;

        double varHeight = h - cardHeight * 1.0;

        for(int i : range(std::min(m_cardsPerColumn, m_deck.size() - m_currentPos)))
        {
            quint32 id = m_deck[i + m_currentPos];

            auto it = newItems.insert(i + m_currentPos, CardItem(id));

            auto &item = it.value();

            int y = varHeight * i / (m_cardsPerColumn - 1);


            m_current = itemAt(mapFromGlobal(QCursor::pos()));
            if(i + m_currentPos == m_current)
            {
                QBrush brush = painter.brush(), newBrush(Qt::lightGray);
                QPen pen = painter.pen();
                painter.setPen(Qt::transparent);
                QColor color(newBrush.color());
                color.setAlpha(160);
                newBrush.setColor(color);
                painter.setBrush(newBrush);
                painter.drawRect(QRect(QPoint(0,  y), QSize(m_sb->geometry().x(), m_cardSize.height())));
                painter.setBrush(brush);
                painter.setPen(pen);
            }

            item.setPos(QPoint(0, y));

            painter.drawPixmap(0, y, cardWidth, cardHeight,
                               *item.getPixmap().data());

            int lim = LimitCards::inst().getLimit(it->getId());
            if(lim < 3)
            {
                auto data = LimitCards::inst().getPixmap(lim);
                if(data)
                {
                    painter.drawPixmap(0, y, 16, 16, *data);
                }
            }

            if(auto ocard = CardManager::inst().getCard(id))
            {
                Card &card = **ocard;

                painter.drawText(cardWidth + 5, y + fmHeight, card.name);
                QString ot;
                QString level = (card.type & Const::TYPE_XYZ) ? "R" : "L";
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
                                     CardManager::inst().getRace(card.id) + "/"
                                     + CardManager::inst().getAttr(card.id) + level);

                    painter.drawText(cardWidth + 5, y + 10 + fmHeight * 3,
                                     adToString(card.atk) + "/" +
                                     adToString(card.def) + ot);
                }
                else if(card.type & (Const::TYPE_SPELL | Const::TYPE_TRAP))
                {
                    painter.drawText(cardWidth + 5, y + 5 + fmHeight * 2,
                                     CardManager::inst().getType(card.id));
                    painter.drawText(cardWidth + 5, y + 10 + fmHeight * 3, ot);
                }
            }
        }
        m_items.swap(newItems);
    }
    if(m_needRefreshId)
    {
        refreshCurrentId();
        m_needRefreshId = false;
    }
    refresh();
}

void CardsList::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        m_startPos = event->pos();
    }
    else if(event->buttons() & Qt::MiddleButton)
    {
        int index = itemAt(event->pos());
        if(index >= 0)
        {
            SignalTower::inst().cardDetails(m_deck[index]);
        }
    }
    QWidget::mousePressEvent(event);
}

int CardsList::itemAt(const QPoint &_pos)
{
    for(int i: range(m_cardsPerColumn))
    {
        if(i + m_currentPos >= m_deck.size())
        {
            break;
        }
        int index = i +  m_currentPos;

        auto &item = m_items.find(index).value();

        if(_pos.y() >= item.getPos().y() &&
                _pos.y() <= item.getPos().y() + m_cardSize.height() &&
                _pos.x() >= 0 && _pos.x() < width())
        {
            return index;
        }
    }
    return -1;
}

void CardsList::refreshCurrentId()
{
    int index = itemAt(m_point);
    if(index != -1)
    {
        quint32 id = m_deck[index];
        if(m_currentCardId != id)
        {
            m_currentCardId = id;
            SignalTower::inst().changeCurrentId(m_deck[index]);
        }
    }
}

void CardsList::mouseMoveEvent(QMouseEvent *event)
{
    int index = itemAt(event->pos());
    if(index != -1)
    {
        quint32 id = m_deck[index];
        if(m_currentCardId != id)
        {
            m_currentCardId = id;
            SignalTower::inst().changeCurrentId(m_deck[index]);
        }
        m_needRefreshId = false;
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

void CardsList::startDrag(int index)
{
    if(index >= m_deck.size())
    {
        return;
    }
    QMimeData *mimedata = new QMimeData;
    quint32 id = m_deck[index];
    mimedata->setText(QString::number(id));
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimedata);
    auto &item = m_items.find(index).value();
    if(item.getPixmap())
    {
        drag->setPixmap(item.getPixmap()->scaled(m_cardSize));
        drag->setHotSpot(QPoint(drag->pixmap().width() / 2, drag->pixmap().height() / 2));
    }
    DragHelper::inst().moved = false;
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
    DragHelper::inst().moved = true;
    event->accept();
}

void CardsList::setPos(int _pos)
{
    int max = m_deck.size() - m_cardsPerColumn;
    max = max > 0 ? max : 0;
    if(m_currentPos > max)
    {
        m_currentPos = max;
    }
    if(_pos >= 0 && _pos <= max)
    {
        m_currentPos = _pos;
        update();
    }
    if(m_sb)
    {
        m_sb->setValue(m_currentPos);
    }
}

void CardsList::setCards(Type::DeckP cards)
{
    m_deck.swap(*cards.data());
    m_deck.squeeze();

    setPos(0);

    emit sizeChanged(m_deck.size());
    refresh();
}

void CardsList::checkLeave()
{
    int i = itemAt(mapFromGlobal(QCursor::pos()));
    if(i != m_current)
    {
        m_current = i;
        update();
    }
}

void CardsList::mouseDoubleClickEvent(QMouseEvent *event)
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
    undoAction->setToolTip(ConfigManager::inst().getStr("action", "undo", ""));
    undoAction->setIcon(QIcon(":/icons/left.png"));
    redoAction = new QAction(this);
    redoAction->setToolTip(ConfigManager::inst().getStr("action", "redo", ""));
    redoAction->setIcon(QIcon(":/icons/right.png"));

    toolbar->addAction(undoAction);
    toolbar->addAction(redoAction);

    auto label = new DeckSizeLabel(ConfigManager::inst().getStr("label", "number", "数目"));
    label->setAlignment(Qt::AlignVCenter |  Qt::AlignRight);

    toolbar->addSeparator();
    auto spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    toolbar->addWidget(spacer);
    toolbar->addWidget(label);

    cl->setScrollBar(sb);

    connect(cl, &CardsList::sizeChanged, label, &DeckSizeLabel::changeSize);
    connect(undoAction, &QAction::triggered, this, &CardsListView::undo);
    connect(redoAction, &QAction::triggered, this, &CardsListView::redo);

    toolbar->setStyleSheet("QToolTip{color: black; font-size: 12px}");
    if(ConfigManager::inst().m_bg)
    {
        cl->setStyleSheet("QWidget{color: white; font-size: 15px}");
        label->setStyleSheet("QLabel{color: white; font-size: 15px}");
    }
    else
    {
        cl->setStyleSheet("QWidget{font-size: 15px}");
        label->setStyleSheet("QLabel{color: black; font-size: 15px}");
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
    undoSnapShots.append(cl->m_deck);
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
    temp.swap(cl->m_deck);
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
    temp.swap(cl->m_deck);
    undoSnapShots.append(std::move(temp));

    auto ptr = Type::DeckP::create();
    ptr->swap(redoSnapShots.back());
    redoSnapShots.pop_back();

    cl->setCards(ptr);
    updateButtons();
}
