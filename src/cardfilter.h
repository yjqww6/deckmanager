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

class CardFilter : public QWidget
{
    Q_OBJECT
public:
    explicit CardFilter(QWidget *parent = 0);
    ~CardFilter();

    std::function<QVector<int>&()> getCurrent;
signals:
    void result(QSharedPointer<QVector<int> >);
public slots:
    void revert();
private slots:
    void setCardTypeSub(int);

    void searchAll();
    void searchThis();
private:

    template<typename T>
    void search(const T&);

    static const int cardTypes[];
    static const int monsterTypes[];
    static const int spellTypes[];
    static const int trapTypes[];
    static const int monsterRaces[];
    static const int monsterAttrs[];

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
    QLineEdit *scaleEdit;

    QCheckBox *effects[32];

    QLineEdit *setEdit;
    QLineEdit *nameEdit;

};

#endif // CARDFILTER_H
