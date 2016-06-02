#ifndef NETWORKING_H
#define NETWORKING_H

#include <QObject>
#include <memory>
#include <unordered_map>
#include "typing.h"
#include "engine.h"

class NetWorking : public QObject
{
    Q_OBJECT
public:
    explicit NetWorking();
    void notify(int type, size_t m_timestamp, ptr data);
    bool getWaiting() const;

    size_t getId() const;

signals:
    void deck(QString);
    void list(Type::DeckL);
    void name(QString);
    void ready(bool);
public slots:
    void getDeck(QString id);
    void getList(int page = 1);
    void getName(quint32);
    void abort();
private:
    size_t m_timestamp;
    size_t m_id;
    bool   m_waiting;
};

std::shared_ptr<NetWorking> make_networking();
void notify(size_t id, int type, int timestamp, ptr data);

#endif // NETWORKING_H
