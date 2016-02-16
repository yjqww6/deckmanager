#include "yrp.h"
#include "range.h"

static void *myAlloc(void *p, std::size_t size)
{
    p = p;
    return new char[size];
}

static void myFree(void *p, void *address)
{
    p = p;
    delete [] static_cast<char*>(address);
}

static ISzAlloc gAlloc = {myAlloc, myFree};

Yrp::Yrp(const QString &path)
{
    QFile file(path);
    if(!file.open(QFile::ReadOnly))
    {
        return;
    }

    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    SizeT compSize = file.size() - sizeof(header);

    if(header.flag & 1)
    {
        QScopedArrayPointer<quint8> data(new quint8[header.datasize]);
        QScopedArrayPointer<quint8> comp_data(new quint8[compSize]);

        file.read(reinterpret_cast<char*>(comp_data.data()), compSize);
        file.close();

        SizeT dataSize = header.datasize;

        ELzmaStatus status;
        if(LzmaDecode(data.data(), &dataSize, comp_data.data(), &compSize,
                      header.props, 5, LZMA_FINISH_ANY, &status, &gAlloc)
                != SZ_OK)
        {
            return;
        }
        loadDecks(data.data());
    }
    else
    {
        QScopedArrayPointer<quint8> data(new quint8[compSize]);
        file.read(reinterpret_cast<char*>(data.data()), compSize);
        loadDecks(data.data());
    }
}

static QString readPlayer(quint8 *data, std::size_t p)
{
    QString name;
    for(int i : range(40))
    {
        Q_UNUSED(i);
        qint16 c = 0;
        c = *(reinterpret_cast<qint16*>(data + p));
        if(c == 0)
        {
            break;
        }
        name.append(c);
        p += 2;
    }
    return name;
}

static std::size_t readDeck(quint8 *_data, std::size_t _p, Type::Deck &deck)
{
    qint32 *data = reinterpret_cast<qint32*>(_data + _p);
    std::size_t p = 0;

    qint32 mainSize = data[p];
    p++;

    deck.reserve(mainSize + 15);

    for(qint32 i : range(mainSize))
    {
        Q_UNUSED(i);
        deck.append(data[p]);
        p++;
    }

    qint32 extraSize = data[p];
    p++;

    for(qint32 i : range(extraSize))
    {
        Q_UNUSED(i);
        deck.append(data[p]);
        p++;
    }
    return p << 2;
}

void Yrp::loadDecks(quint8 *data)
{
    std::size_t p = 0;
    int players = header.flag & 2 ? 4 : 2;

    for(int i : range(players))
    {
        Q_UNUSED(i);
        decks.append(qMakePair(readPlayer(data, p), Type::Deck()));
        p += 40;
    }

    p += 4 * sizeof(qint32);

    for(int i : range(2))
    {
        p += readDeck(data, p, decks[i].second);
    }

    if(players == 4)
    {
        p += readDeck(data, p, decks[3].second);
        p += readDeck(data, p, decks[2].second);
    }
}

Yrp::~Yrp()
{

}

