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
        int xoff = (width() - cardSize.width()) / 2;
        painter.drawPixmap(QRect(QPoint(xoff, 0), cardSize), *item.getPixmap().data());
    }
}

QSize CardPicture::calcSize(QSize s)
{
    double timesH = s.height() * 1.0 / 254, timesW = s.width() * 1.0 / 177;
    double times = std::min(timesH, timesW);
    if(times <= 1.0 || !item.getPixmap())
    {
        return QSize(177, 254);
    }
    else
    {
        QSize temp = QSize(177 * timesW, 254 * timesH);
        if(temp.width() > item.getPixmap()->width() ||
                temp.height() > item.getPixmap()->height())
        {
            return item.getPixmap()->size();
        }
        return temp;
    }
}

void CardPicture::resizeEvent(QResizeEvent *event)
{
    cardSize = calcSize(size());
    QWidget::resizeEvent(event);
}

void CardPicture::setId(int id)
{
    item = CardItem(id);
    update();
}

CardPicture::~CardPicture()
{

}

