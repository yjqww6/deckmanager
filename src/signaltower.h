#ifndef SIGNALTOWER_H
#define SIGNALTOWER_H

#include <QObject>
#include "common.h"

class SignalTower : public QObject, public enable_singleton<SignalTower>
{
    Q_OBJECT
public:
    explicit SignalTower(QObject *parent = 0) : QObject(parent), m_mainLoaded(false)
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

    void schemeDebug(QString str)
    {
        emit debug(str);
    }

signals:
    void clickId(quint32);
    void currentIdChanged(quint32);
    void details(quint32);
    void debug(QString str);
public slots:

public:
    QString m_accumulated;
    bool m_mainLoaded;
};
#endif // SIGNALTOWER_H
