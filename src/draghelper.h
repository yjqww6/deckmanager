#ifndef DRAGHELPER_H
#define DRAGHELPER_H

#include <QObject>
#include <QHash>
#include "carditem.h"

class DragHelper : public QObject
{
    Q_OBJECT
public:
    explicit DragHelper(QObject *parent = 0);
    bool moved;
    bool atomic;

signals:

public slots:

protected:
};

extern DragHelper dragHelper;

#endif // DRAGHELPER_H
