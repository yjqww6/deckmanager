#ifndef DECKWIDGET_H
#define DECKWIDGET_H

#include <QWidget>
#include <QList>
#include <QPainter>
#include <QPoint>
#include <QMouseEvent>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QMimeData>
#include <functional>
#include "carditem.h"
#include "card.h"
#include "limitcards.h"

class DeckWidget : public QWidget
{
    Q_OBJECT
protected:
    int itemAt(const QPoint &pos);
    int posIndex(const QPoint &pos);

    void startDrag(int);

    Type::DeckI &deck;
    QPoint startPos;
    QSize cardSize;
    int row;
    int column;
    QSize offset;
    QSize spacing;
    quint32 currentCardId;

    int deckSize;
    int current;
public:
    explicit DeckWidget(QWidget *parent, int _row, int _column, Type::DeckI &_deck);
    ~DeckWidget();
    void paintEvent(QPaintEvent *event);


    auto getDeck() -> decltype((deck))
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

    void addCard(quint32 id)
    {
        deck.append(CardItem(id));
    }

    void insertCard(int index, quint32 id)
    {
        deck.insert(index, CardItem(id));
    }

    void clearDeck()
    {
        deck.clear();
    }

    int countCard(quint32 id);

    void deleteCard(QPoint);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

    void mouseDoubleClickEvent(QMouseEvent *event);

    void setCurrentCardId(quint32 id)
    {
        currentCardId = id;
    }

    std::function<bool(quint32)> filter;
    std::function<bool(quint32)> extFilter;
    std::function<void()> makeSnapShot;
    bool overlapV;
signals:
    void currentIdChanged(quint32 id);
    void sizeChanged(int size);
    void deckChanged(Type::DeckI&);
    void clickId(quint32);
    void details(quint32);
public slots:
    void checkLeave();
};

#endif // DECKWIDGET_H
