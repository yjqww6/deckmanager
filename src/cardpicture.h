#ifndef CARDPICTURE_H
#define CARDPICTURE_H

#include <QWidget>
#include <QPainter>
#include "carditem.h"

class CardPicture : public QWidget
{
    Q_OBJECT
public:
    explicit CardPicture(QWidget *parent = 0);
    ~CardPicture();

    void paintEvent(QPaintEvent *);
signals:

public slots:
    void setId(int);

private:
    CardItem item;
};

#endif // CARDPICTURE_H
