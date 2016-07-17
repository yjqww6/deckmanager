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
#include "types.h"

class DeckModel;

class DeckWidget : public QWidget
{
    Q_OBJECT
protected:
    int itemAt(const QPoint &pos);
    int posIndex(const QPoint &pos);

    void startDrag(int);
public:
    explicit DeckWidget(QWidget *parent, int _row, int _column,
                        QSharedPointer<Type::DeckI> _deck,  QSharedPointer<DeckModel> _model);
    ~DeckWidget();
    void paintEvent(QPaintEvent *event);

    Type::DeckI& getDeck()
    {
        return *m_deck;
    }

    QSize getCardSize()
    {
        return m_cardSize;
    }

    void setCardSize(const QSize &_size)
    {
        m_cardSize = _size;
    }

    void addCard(quint32 id)
    {
        m_deck->append(CardItem(id));
    }

    void insertCard(int index, quint32 id)
    {
        m_deck->insert(index, CardItem(id));
    }

    void clearDeck()
    {
        m_deck->clear();
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
        m_currentCardId = id;
    }
    void setDeck(const QSharedPointer<Type::DeckI> value);

signals:
    void sizeChanged(int size);
    void deckChanged(Type::DeckI&);

protected:
    QSharedPointer<Type::DeckI> m_deck;
    QPoint m_startPos;
    QSize m_cardSize;
    int m_row;
    int m_column;
    QSize m_offset;
    QSize m_spacing;
    quint32 m_currentCardId;

    int m_deckSize;
    int m_current;
public:


    std::function<bool(quint32)> m_filter;
    std::function<bool(quint32)> m_extFilter;
    std::function<void()> m_makeSnapShot;
    bool m_overlapV;
    QSharedPointer<DeckModel> m_currentModel;

};

#endif // DECKWIDGET_H
