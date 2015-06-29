#ifndef DECKWIDGET_H
#define DECKWIDGET_H
#include <QApplication>
#include <QWidget>
#include <QList>
#include <QPainter>
#include <QPoint>
#include <QMouseEvent>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QMimeData>
#include <QSharedPointer>
#include <algorithm>
#include <functional>
#include "carditem.h"
#include "card.h"
#include "limitcards.h"

class DeckWidget : public QWidget
{
    Q_OBJECT
public:
    explicit DeckWidget(QWidget *parent, int _row, int _column, bool &, bool &);
    ~DeckWidget();
    void paintEvent(QPaintEvent *event);


    QList<CardItem>& getDeck()
    {
        return deck;
    }

    QSize getCardSize()
    {
        return cardSize;
    }

    void setCardSize(const QSize &_size)
    {
        cardSize = _size;
    }

    void addCard(int id)
    {
        deck.append(std::move(CardItem(id)));
    }

    void insertCard(int index, int id)
    {
        deck.insert(index, std::move(CardItem(id)));
    }

    void clearDeck()
    {
        deck.clear();
    }

    int countCard(int id);

    void deleteCard(QPoint);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

    void mouseDoubleClickEvent(QMouseEvent *event)
    {
        if(event->buttons() & Qt::LeftButton)
        {
            emit clickId(currentCardId);
        }
        QWidget::mouseDoubleClickEvent(event);
    }

    virtual bool filter(int id)
    {
        auto card = CardPool::getCard(id);
        return card;
    }

    void setCurrentCardId(int id)
    {
        currentCardId = id;
    }


    std::function<bool(int)> extFilter;
    std::function<void()> makeSnapShot;
signals:
    void currentIdChanged(int id);
    void sizeChanged(int size);
    void deckChanged(QList<CardItem> &);
    void clickId(int);
    void details(int);
public slots:
    void shuffle();
    void sort();
    void checkLeave();
private:
    int itemAt(const QPoint &pos);
    int posIndex(const QPoint &pos);

    void startDrag(int);

    QList<CardItem> deck;
    QPoint startPos;
    QSize cardSize;
    int row;
    int column;
    QSize offset;
    QSize spacing;
    int currentCardId;

    int deckSize;
    int current;
    bool &isUs;
    bool &moved;
};

#endif // DECKWIDGET_H
