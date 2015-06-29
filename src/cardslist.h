#ifndef CARDSVIEW_H
#define CARDSVIEW_H

#include <QWidget>
#include <QVector>
#include <QPainter>
#include <QPoint>
#include <QFontMetrics>
#include <QSharedPointer>
#include <QApplication>
#include <QMimeData>
#include <QMouseEvent>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QWheelEvent>
#include <QtSql/QSqlQuery>
#include "card.h"
#include "carditem.h"
#include "deckview.h"

class CardsList : public QWidget
{
    Q_OBJECT
public:
    explicit CardsList(QWidget *parent, bool &);
    ~CardsList();

    QVector<int> &getList()
    {
        return ls;
    }

    void paintEvent(QPaintEvent *);

    void wheelEvent(QWheelEvent *);

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    void dragEnterEvent(QDragEnterEvent *event);
    void dragMoveEvent(QDragMoveEvent *event);
    void dropEvent(QDropEvent *event);

    void setScrollBar(QScrollBar *);
    void setCurrentCardId(int id)
    {
        currentCardId = id;
    }

    void refresh();

    void mouseDoubleClickEvent(QMouseEvent *event)
    {
        if(event->buttons() & Qt::LeftButton)
        {
            emit clickId(currentCardId);
        }
        QWidget::mouseDoubleClickEvent(event);
    }

signals:
    void sizeChanged(int);
    void currentIdChanged(int id);
    void clickId(int);
    void details(int);
public slots:
    void setCards(QSharedPointer<QVector<int> >);
    void checkLeave();

private slots:
    void refreshCurrentId();
    void setPos(int _pos);

protected:
    int itemAt(const QPoint &pos);

    QString adToString(int);
    virtual void startDrag(int);

    QVector<int> ls;
    QHash<int, CardItem> items;

    int pos;
    QSize cardSize;
    QPoint startPos;
    int cardsPerColumn;
    int currentCardId;
    bool &moved;
    QScrollBar *sb;
    bool needRefreshId;
    QPoint point;
    int current;
};

class CardsListView : public QWidget
{
    Q_OBJECT
public:
    CardsListView(QWidget *parent, bool &);
    QVector<int> &getList()
    {
        return cl->getList();
    }

    void refresh()
    {
        cl->refresh();
    }

signals:
    void currentIdChanged(int id);
    void clickId(int);
    void details(int);
public slots:

    void setCards(QSharedPointer<QVector<int> > cards)
    {
        cl->setCards(cards);
    }

    void setCurrentCardId(int id)
    {
        cl->setCurrentCardId(id);
    }

    void checkLeave()
    {
        cl->checkLeave();
    }

private slots:
    void changeId(int id)
    {
        emit currentIdChanged(id);
    }

    void idClicked(int id)
    {
        emit clickId(id);
    }

private:
    CardsList *cl;
};

#endif // CARDSVIEW_H
