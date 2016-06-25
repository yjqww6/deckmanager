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
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    if(m_item.getId() && m_item.getPixmap())
    {
        painter.drawPixmap(QRect(QPoint(0, 0), cardSize), *m_item.getPixmap().data());
    }
}

void CardPicture::setId(quint32 id, int maxWidth)
{
    m_item = CardItem(id);
    if(m_item.getPixmap() && m_item.getPixmap()->height() > 0)
    {
        if(m_item.getPixmap()->width() > maxWidth)
        {
            QSize size(maxWidth, maxWidth * 254 / 177);
            setFixedSize(size);
            cardSize = size;
        }
        else
        {
            setFixedSize(m_item.getPixmap()->size());
            cardSize = m_item.getPixmap()->size();
        }
    }
    update();
}

CardPicture::~CardPicture()
{

}

