#ifndef PREF_H
#define PREF_H

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include "typing.h"

class Pref : public QWidget
{
    Q_OBJECT
public:
    explicit Pref(QWidget *parent = 0);

signals:
    void lflistChanged();
    void lfList(Type::DeckP);
public slots:
    void setLflist(int index);
    void openLfList();
private:
    QComboBox *lfcombo;
};

#endif // PREF_H
