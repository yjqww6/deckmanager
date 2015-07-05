#ifndef YRP_H
#define YRP_H
#include <QString>
#include <QFile>
#include <QVector>
#include <QList>
#include <QPair>
#include <QScopedArrayPointer>
#include "lzma/LzmaDec.h"
#include "typing.h"

struct ReplayHeader
{
    unsigned int id;
    unsigned int version;
    unsigned int flag;
    unsigned int seed;
    unsigned int datasize;
    unsigned int hash;
    unsigned char props[8];
};

class Yrp
{
public:
    Yrp(const QString&);
    ~Yrp();
    typedef QPair<QString, Type::Deck> Item;

    QList<Item> decks;

private:
    void loadDecks(quint8 *data);

    ReplayHeader header;
};

#endif // YRP_H
