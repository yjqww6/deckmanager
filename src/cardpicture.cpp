#include "cardpicture.h"
#include <QDebug>

CardPicture::CardPicture(QWidget *parent)
    : QWidget(parent), cardSize(177, 254)
{
    setMinimumSize(QSize(177, 254));
}

void CardPicture::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if(item.getId() && item.getPixmap())
    {
        painter.drawPixmap(QRect(QPoint(0, 0), cardSize), *item.getPixmap().data());
    }
}

void CardPicture::setId(int id)
{
    item = CardItem(id);
    if(item.getPixmap() && item.getPixmap()->height() > 0)
    {
        setFixedSize(item.getPixmap()->size());
        cardSize = item.getPixmap()->size();
    }
    update();
}

CardPicture::~CardPicture()
{

}

