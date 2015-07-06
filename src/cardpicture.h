#ifndef CARDPICTURE_H
#define CARDPICTURE_H

#include <QWidget>
#include <QPainter>
#include <QResizeEvent>
#include "carditem.h"

class CardPicture : public QWidget
{
    Q_OBJECT
public:
    explicit CardPicture(QWidget *parent = 0);
    ~CardPicture();
    void paintEvent(QPaintEvent *);
    QSize cardSize;
signals:

public slots:
    void setId(quint32, int);

private:
    CardItem item;
};

#endif // CARDPICTURE_H
