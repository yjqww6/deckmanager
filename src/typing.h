#ifndef TYPING_H
#define TYPING_H

#include <QVector>
#include <QSharedPointer>

namespace Type
{
typedef QVector<quint32> Deck;
typedef QSharedPointer<Deck> DeckP;
}
#endif // TYPING_H

