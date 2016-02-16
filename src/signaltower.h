#ifndef SIGNALTOWER_H
#define SIGNALTOWER_H

#include <QObject>

class SignalTower : public QObject
{
    Q_OBJECT
public:
    explicit SignalTower(QObject *parent = 0) : QObject(parent)
    {

    }

    void IdClick(quint32 id)
    {
        emit clickId(id);
    }

    void changeCurrentId(quint32 id)
    {
        emit currentIdChanged(id);
    }

    void cardDetails(quint32 id)
    {
        emit details(id);
    }

signals:
    void clickId(quint32);
    void currentIdChanged(quint32);
    void details(quint32);
public slots:
};

extern SignalTower *tower;
#endif // SIGNALTOWER_H
