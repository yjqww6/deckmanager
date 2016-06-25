#include "carddetails.h"
#include "configmanager.h"
#include "engine.h"
#include <QVBoxLayout>

CardDetails::CardDetails(QWidget *parent)
    : QWidget(parent), m_offset(3), m_currentId(0)
{
    m_cp = new CardPicture;
    m_effect = new QPlainTextEdit;
    m_effect->setReadOnly(true);

    m_vbox = new QVBoxLayout;
    m_vbox->addWidget(m_cp);
    m_vbox->addWidget(m_effect, 1);
    setLayout(m_vbox);
}

void CardDetails::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(m_currentId != 0)
    {
        emit clickId(m_currentId);
    }
    QWidget::mouseDoubleClickEvent(event);
}

void CardDetails::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    setId(m_currentId);
}

void CardDetails::setId(quint32 id)
{
    if(id != m_currentId)
    {
        m_currentId = id;

        with_scheme([=]()
        {
            ptr str = engine->call("detail-string", Sunsigned32(id));

            m_effect->clear();
            m_effect->insertPlainText(engine->getString(str));
        });

        m_cp->setId(id, m_effect->width());
    }
    updateGeometry();
}

CardDetails::~CardDetails()
{

}

