#ifndef LOCALLIST_H
#define LOCALLIST_H

#include <QWidget>
#include <QListWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTextStream>
#include <QDir>

class LocalList : public QWidget
{
    Q_OBJECT
public:
    explicit LocalList(QWidget *parent = 0);
    ~LocalList();

signals:
    void deckStream(QString, QString, bool);
    void saveDeck(QString);
public slots:
    void refresh();
    void setPathFocus()
    {
        pathEdit->setFocus();
        pathEdit->selectAll();
    }

private slots:
    void deckStreamTrans(QListWidgetItem *);
    void saveDeckTrans();
    void itemName(QListWidgetItem *);
private:
    QListWidget *listWidget;
    QLineEdit *pathEdit;
    QPushButton *buttonSave;
    QPushButton *buttonRefresh;
};

#endif // LOCALLIST_H
