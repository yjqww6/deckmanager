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
    CardPicture *cp;
    QPlainTextEdit *effect;
    QVBoxLayout *vbox;
    int offset;
    quint32 currentId;
};

#endif // CARDDETAILS_H
