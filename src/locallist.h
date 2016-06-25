#ifndef LOCALLIST_H
#define LOCALLIST_H

#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextStream>
#include <QMenu>
#include <QCheckBox>
#include <QDir>

class LocalList : public QWidget
{
    Q_OBJECT
public:
    explicit LocalList(QWidget *parent = 0);
    ~LocalList();

    void contextMenuEvent(QContextMenuEvent *);
signals:
    void deckStream(QString, QString, bool, bool);
    void saveDeck(QString);
public slots:
    void refresh();
    void setPathFocus(QString name)
    {
        m_pathEdit->setText(name);
        m_pathEdit->setFocus();
        m_pathEdit->selectAll();
    }

private slots:
    void sendDeck(QListWidgetItem *, bool);
    void saveDeckTrans();
    void itemName(QListWidgetItem *);
    void deleteDeck();
private:
    QListWidget     *m_listWidget;
    QLineEdit       *m_pathEdit;
    QPushButton     *m_buttonSave;
    QPushButton     *m_buttonRefresh;
    QListWidgetItem *m_menuItem;
    QCheckBox       *m_includeAI;
    QMenu           *m_popup;
};

#endif // LOCALLIST_H
