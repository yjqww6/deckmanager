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
#include "typing.h"
#include <functional>

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
    class Matcher
    {
    private:
        std::function<bool(int)> pred;
        bool valid;
    public:
        template<typename T>
        Matcher(T _pred, bool _valid = true) : pred(_pred), valid(_valid) {}
        bool isValid() const
        {
            return valid;
        }
        bool operator ()(int p) const
        {
            return pred(p);
        }
    };

    typedef std::function<bool(Card&)> Pred;
    typedef std::function<void(Pred&&)> Ctx;

    void call_with_pred(Ctx &&ctx);

    template<typename T>
    void search(T &&begin, T &&end, Pred &&predicate);

    static const quint32 cardTypes[];
    static const quint32 monsterTypes[];
    static const quint32 spellTypes[];
    static const quint32 trapTypes[];
    static const quint32 monsterRaces[];
    static const quint32 monsterAttrs[];

    Matcher rangeMatcher(QString);
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
    QCheckBox *inverseMode;
    QCheckBox *noSortMode;
};

#endif // CARDFILTER_H
