#ifndef CARDDETAILS_H
#define CARDDETAILS_H

#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
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
    void resizeEvent(QResizeEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
signals:
    void clickId(int);
public slots:
    void setId(int);

private:
    CardPicture *cp;
    QLabel *name;
    QLabel *type;
    QLabel *set;
    QLabel *ad;
    QLabel *effect;
    int offset;
    int currentId;
};

#endif // CARDDETAILS_H
