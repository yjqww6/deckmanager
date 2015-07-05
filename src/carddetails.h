#ifndef CARDDETAILS_H
#define CARDDETAILS_H

#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPlainTextEdit>
#include <QFont>
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
signals:
    void clickId(quint32);
public slots:
    void setId(quint32);

private:
    CardPicture *cp;
    QPlainTextEdit *effect;
    QVBoxLayout *vbox;
    int offset;
    int currentId;
};

#endif // CARDDETAILS_H
