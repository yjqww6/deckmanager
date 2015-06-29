#ifndef EXPANSIONS_H
#define EXPANSIONS_H

#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include <QStringList>
#include <QList>
#include <QByteArray>

class Expansions
{
public:
    Expansions(QStringList zipFiles);

    QByteArray open(QString path);
private:
    QStringList zips;
};


extern Expansions *expansions;

#endif // EXPANSIONS_H
