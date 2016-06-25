#ifndef PACKVIEW_H
#define PACKVIEW_H

#include <QWidget>
#include <QTabWidget>
#include <QPushButton>
#include "packlist.h"
#include "packedit.h"
#include "types.h"

class PackView : public QWidget
{
    Q_OBJECT
public:
    explicit PackView(QWidget *parent);
    void refresh();

signals:
    void cards(Type::DeckP);
    void details(quint32);
    void clickId(quint32);
    void checkLeave();
public slots:
    void changeTab();

private:
    PackList        *m_packList;
    RemotePackList  *m_remotePackList;
    QTabWidget      *m_tab;
};

#endif // PACKVIEW_H
