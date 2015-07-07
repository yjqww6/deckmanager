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
}
#endif // TYPING_H

