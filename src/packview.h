#ifndef PACKVIEW_H
#define PACKVIEW_H

#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include "packlist.h"
#include "packedit.h"

class PackView : public QWidget
{
    Q_OBJECT
public:
    explicit PackView(QWidget *parent);
    void refresh()
    {
        packList->refresh();
    }

signals:
    void cards(QSharedPointer<QVector<int> >);
    void details(int);

    void checkingLeave();
public slots:
    void changeTab()
    {
        tab->setCurrentIndex(1);
    }

private:
    PackList *packList;
    QTabWidget *tab;
};

#endif // PACKVIEW_H
