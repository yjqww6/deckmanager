#include "carddetails.h"
#include "config.h"
#include "wrapper.h"
#include "engine.h"
#include <QVBoxLayout>

CardDetails::CardDetails(QWidget *parent)
    : QWidget(parent), offset(3), currentId(0)
{
    cp = new CardPicture;
    effect = new QPlainTextEdit;
    effect->setReadOnly(true);

    vbox = new QVBoxLayout;
    vbox->addWidget(cp);
    vbox->addWidget(effect, 1);
    setLayout(vbox);
}

void CardDetails::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(currentId != 0)
    {
        emit clickId(currentId);
    }
    QWidget::mouseDoubleClickEvent(event);
}

void CardDetails::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    setId(currentId);
}

void CardDetails::setId(quint32 id)
{
    if(id != currentId)
    {
        currentId = id;

        with_scheme([=]()
        {
            ptr str = engine->call("detail-string", Sunsigned32(id));

            effect->clear();
            effect->insertPlainText(QString::fromUtf8(engine->getString(str).c_str()));
        });

        cp->setId(id, effect->width());
    }
    updateGeometry();
}

CardDetails::~CardDetails()
{

}

