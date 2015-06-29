#include "carddetails.h"
#include "config.h"

CardDetails::CardDetails(QWidget *parent)
    : QWidget(parent), offset(3), currentId(0)
{
    setMinimumWidth(180);
    int fmHeight = fontMetrics().height() + 3;
    int w = width()  - offset * 2;
    int y = 0;

    cp = new CardPicture(this);

    name = new QLabel(this);
    type = new QLabel(this);
    set = new QLabel(this);
    ad = new QLabel(this);
    effect = new QLabel(this);
    effect->setWordWrap(true);
    effect->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    cp->setGeometry(1 + (width() - 177) / 2, 0, width(), 254);

    y += 260;

    name->setGeometry(offset, y, w, fmHeight);

    y += fmHeight;
    type->setGeometry(offset, y, w, fmHeight);

    y += fmHeight;
    set->setGeometry(offset, y, w, fmHeight);

    y += fmHeight;
    ad->setGeometry(offset, y, w, fmHeight);


    y += fmHeight + 5;
    effect->setGeometry(offset, y, w, height() - y);

}

void CardDetails::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit clickId(currentId);
    QWidget::mouseDoubleClickEvent(event);
}

void CardDetails::resizeEvent(QResizeEvent *)
{

    int fmHeight = fontMetrics().height() + 3;
    int w = width()  - offset * 2;

    int y = 0;
    cp->setGeometry(1 + (width() - 177) / 2, 0, width(), 254);

    y += 260;

    name->setGeometry(offset, y, w, fmHeight);

    y += fmHeight;
    type->setGeometry(offset, y, w, fmHeight);

    y += fmHeight;
    set->setGeometry(offset, y, w, fmHeight);

    y += fmHeight;
    ad->setGeometry(offset, y, w, fmHeight);


    y += fmHeight + 5;
    effect->setGeometry(offset, y, w, height() - y);
}


void CardDetails::setId(int id)
{
    QSharedPointer<Card> card = CardPool::getCard(id);
    name->setText(card->name + "[" + QString::number(id) + "]");
    currentId = id;

    QString ot;
    if((card->ot & 0x3) == 1)
    {
        ot = tr("[OCG]");
    }
    else if((card->ot & 0x3) == 2)
    {
        ot = tr("[TCG]");
    }

    if(card->type & Card::TYPE_MONSTER)
    {
        type->setText("[" + card->cardType() + "]"
                      + card->cardRace() + "/" + card->cardAttr());
        QString level = (card->type & Card::TYPE_XYZ) ? tr("R") : tr("L");
        level = "[" + level + QString::number(card->level) + "]";
        ad->setText(level + card->cardAD() + ot);
    }
    else
    {
        type->setText("[" + card->cardType() + "]");
        ad->setText(ot);
    }

    quint64 setcode = card->setcode;
    QStringList setcodeStr;
    while(setcode)
    {
        QString cur;
        cur.setNum(setcode & 0xffff, 16);
        setcodeStr << cur;
        setcode = setcode >> 16;
    }
    set->setText(config->getStr("label", "setcode", "系列") + ":[" + setcodeStr.join(tr("|")) + "]");

    effect->setText(card->effect);
    cp->setId(id);
}

CardDetails::~CardDetails()
{

}

