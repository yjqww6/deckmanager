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
        m_deck.clear();
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
public:
    PackEditView(QWidget *parent);
    Type::Deck& getList()
    {
        return m_pe->m_deck;
    }

    void refresh()
    {
        m_pe->refresh();
    }
signals:
    void refreshPack();
    void saved();
public slots:

    void setCards(Type::DeckP cards)
    {
        m_pe->setCards(cards);
    }

    void setCurrentCardId(quint32 id)
    {
        m_pe->setCurrentCardId(id);
    }

    void checkLeave()
    {
        m_pe->checkLeave();
    }


    void setName(QString name)
    {
        m_nameEdit->setText(name);
    }
private slots:
    void saveList()
    {
        m_pe->saveList(m_nameEdit->text());
        emit refreshPack();
    }

private:
    PackEdit *m_pe;
    QLineEdit *m_nameEdit;
};

#endif // PACKEDIT_H
