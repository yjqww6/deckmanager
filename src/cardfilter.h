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
#include "types.h"
#include "engine.h"
#include <functional>

class CardFilter : public QWidget
{
    Q_OBJECT
public:
    explicit CardFilter(QWidget *parent = 0);
    ~CardFilter();
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
    ptr make_pred();

    template<typename T>
    void search(T &&begin, T &&end);

public:
    std::function<Type::Deck&()> m_getCurrent;
    std::function<Type::DeckP()> m_getDeck;

private:
    static const quint32 cardTypes[];
    static const quint32 monsterTypes[];
    static const quint32 spellTypes[];
    static const quint32 trapTypes[];
    static const quint32 monsterRaces[];
    static const quint32 monsterAttrs[];

    QComboBox *m_cardType;
    QComboBox *m_cardTypeSub;

    QComboBox *m_limit, *m_ot;

    QLineEdit *m_passEdit;

    QComboBox *m_cardRace;
    QComboBox *m_cardAttr;

    QLineEdit *m_atkEdit;
    QLineEdit *m_defEdit;
    QLineEdit *m_levelEdit;
    QLineEdit *m_rankEdit;
    QLineEdit *m_scaleEdit;

    QCheckBox *m_effects[32];

    QComboBox *m_setEdit;
    QLineEdit *m_nameEdit;
    QCheckBox *m_inverseMode;
    QCheckBox *m_noSortMode;
};

#endif // CARDFILTER_H
