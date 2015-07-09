#ifndef CARDFILTER_H
#define CARDFILTER_H

#include <QWidget>
#include <QScrollArea>
#include <QGridLayout>
#include <QTabWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>
#include <QCheckBox>
#include <QPushButton>
#include <QVector>
#include "card.h"
#include <functional>
#include "typing.h"

class CardFilter : public QWidget
{
    Q_OBJECT
public:
    explicit CardFilter(QWidget *parent = 0);
    ~CardFilter();

    std::function<Type::Deck&()> getCurrent;
    std::function<Type::DeckP()> getDeck;
signals:
    void result(Type::DeckP);
public slots:
    void revert();
    void searchSet(quint32 id);
private slots:
    void setCardTypeSub(int);

    void searchAll();
    void searchThis();
    void searchDeck();
private:

    template<typename T>
    void search(const T&);

    static const quint32 cardTypes[];
    static const quint32 monsterTypes[];
    static const quint32 spellTypes[];
    static const quint32 trapTypes[];
    static const quint32 monsterRaces[];
    static const quint32 monsterAttrs[];

    QPair<int, int> getRange(QLineEdit*);
    bool matchRange(QPair<int, int>, int);
    QComboBox *cardType;
    QComboBox *cardTypeSub;

    QComboBox *limit, *ot;

    QLineEdit *passEdit;

    QComboBox *cardRace;
    QComboBox *cardAttr;

    QLineEdit *atkEdit;
    QLineEdit *defEdit;
    QLineEdit *levelEdit;
    QLineEdit *rankEdit;
    QLineEdit *scaleEdit;

    QCheckBox *effects[32];

    QLineEdit *setEdit;
    QLineEdit *nameEdit;

};

#endif // CARDFILTER_H
