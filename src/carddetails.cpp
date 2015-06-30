#include "carddetails.h"
#include "config.h"

CardDetails::CardDetails(QWidget *parent)
    : QWidget(parent), offset(3), currentId(0)
{
    setMinimumWidth(180);

    cp = new CardPicture(this);
    effect = new QPlainTextEdit(this);
    effect->setReadOnly(true);
}

void CardDetails::mouseDoubleClickEvent(QMouseEvent *event)
{
    emit clickId(currentId);
    QWidget::mouseDoubleClickEvent(event);
}

void CardDetails::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    int w = width()  - offset * 2;

    int y = 0;
    QSize cpSize = cp->calcSize(QSize(width(), width() * 254 / 177));

    cp->setGeometry(0, 0, width(), cpSize.height());
    y += cpSize.height() + 4;

    effect->setGeometry(offset, y, w, height() - y);
}


void CardDetails::setId(int id)
{
    QStringList str;
    QSharedPointer<Card> card = CardPool::getCard(id);
    str << card->name + "[" + QString::number(id) + "]";
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
        str << ("[" + card->cardType() + "]"
                      + card->cardRace() + "/" + card->cardAttr());
        QString level = (card->type & Card::TYPE_XYZ) ? tr("R") : tr("L");
        level = "[" + level + QString::number(card->level) + "]";
        str << (level + card->cardAD() + ot);
    }
    else
    {
        str << ("[" + card->cardType() + "]");
        str << (ot);
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
    str << (config->getStr("label", "setcode", "系列") + ":[" + setcodeStr.join(tr("|")) + "]");
    str << card->effect;
    effect->clear();
    effect->insertPlainText(str.join('\n'));
    cp->setId(id);
    updateGeometry();
}

CardDetails::~CardDetails()
{

}

