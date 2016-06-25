#ifndef CARDDETAILS_H
#define CARDDETAILS_H

#include <QWidget>
#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QResizeEvent>
#include "cardpicture.h"
#include "card.h"

class CardDetails : public QWidget
{
    Q_OBJECT
public:
    explicit CardDetails(QWidget *parent = 0);
    ~CardDetails();

    void mouseDoubleClickEvent(QMouseEvent *);
    void resizeEvent(QResizeEvent *);
signals:
    void clickId(quint32);
public slots:
    void setId(quint32);

private:
    CardPicture     *m_cp;
    QPlainTextEdit  *m_effect;
    QVBoxLayout     *m_vbox;
    int             m_offset;
    quint32         m_currentId;
};

#endif // CARDDETAILS_H
