#ifndef PACKEDIT_H
#define PACKEDIT_H
#include "cardslist.h"
#include <QDropEvent>
#include <QLineEdit>
#include <QDir>
#include <QPushButton>

class PackEdit : public CardsList
{
    Q_OBJECT
public:
    PackEdit(QWidget *parent);
    void mousePressEvent(QMouseEvent *);
    void dropEvent(QDropEvent *);
    void dragEnterEvent(QDragEnterEvent *);
    virtual void startDrag(int);
signals:
    void saved();
public slots:

    void sort();
    void saveList(QString);
    void clearList()
    {
        ls.clear();
        emit sizeChanged(0);
        update();
    }

private:
    int itemAt2(const QPoint);
    bool filter(quint32 id);
    int posIndex(QPoint);
};

class PackEditView : public QWidget
{
    Q_OBJECT
private:
    PackEdit *pe;
    QLineEdit *nameEdit;
public:
    PackEditView(QWidget *parent);
    auto getList() -> decltype(pe->getList())
    {
        return pe->getList();
    }

    void refresh()
    {
        pe->refresh();
    }
signals:
    void refreshPack();
    void saved();
public slots:

    void setCards(Type::DeckP cards)
    {
        pe->setCards(cards);
    }

    void setCurrentCardId(quint32 id)
    {
        pe->setCurrentCardId(id);
    }

    void checkLeave()
    {
        pe->checkLeave();
    }


    void setName(QString name)
    {
        nameEdit->setText(name);
    }
private slots:
    void saveList()
    {
        pe->saveList(nameEdit->text());
        emit refreshPack();
    }
};

#endif // PACKEDIT_H
