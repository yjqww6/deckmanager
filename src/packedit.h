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
    PackEdit(QWidget *parent, bool &);
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
    bool filter(int id);
    int posIndex(QPoint);
};

class PackEditView : public QWidget
{
    Q_OBJECT
public:
    PackEditView(QWidget *parent, bool &);
    QVector<int> &getList()
    {
        return pe->getList();
    }

    void refresh()
    {
        pe->refresh();
    }
signals:
    void currentIdChanged(int id);
    void clickId(int);
    void refreshPack();
    void saved();
    void details(int);
public slots:

    void setCards(QSharedPointer<QVector<int> > cards)
    {
        pe->setCards(cards);
    }

    void setCurrentCardId(int id)
    {
        pe->setCurrentCardId(id);
    }

    void checkLeave()
    {
        pe->checkLeave();
    }

private slots:
    void changeId(int id)
    {
        emit currentIdChanged(id);
    }

    void idClicked(int id)
    {
        emit clickId(id);
    }

    void saveList()
    {
        pe->saveList(nameEdit->text());
        emit refreshPack();
    }

    void setName(QString name)
    {
        nameEdit->setText(name);
    }

private:
    PackEdit *pe;
    QLineEdit *nameEdit;
};

#endif // PACKEDIT_H
