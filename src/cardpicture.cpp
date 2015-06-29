#include "cardpicture.h"

CardPicture::CardPicture(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(QSize(177, 254));
}

void CardPicture::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    if(item.getId())
    {
        painter.drawPixmap(0, 0, 177, 254, *item.getPixmap().data());
    }
}

void CardPicture::setId(int id)
{
    item = CardItem(id);
    update();
}

CardPicture::~CardPicture()
{

}

