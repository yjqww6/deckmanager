#ifndef TYPING_H
#define TYPING_H

#include <QVector>
#include <QSharedPointer>
#include "carditem.h"

namespace Type
{
typedef QVector<quint32> Deck;
typedef QSharedPointer<Deck> DeckP;
typedef QList<CardItem> DeckI;
typedef QSharedPointer<QList<QPair<QString, QVariantList> > > DeckL;
}
#endif // TYPING_H

