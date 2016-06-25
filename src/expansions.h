#ifndef EXPANSIONS_H
#define EXPANSIONS_H

#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include <QStringList>
#include <QList>
#include <QByteArray>
#include "common.h"

class Expansions : public enable_singleton<Expansions>
{
public:
    void loadZips(QStringList zipFiles);
    QByteArray open(QString path);
private:
    QList<QSharedPointer<QuaZip> > m_zips;
};

#endif // EXPANSIONS_H
