#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "deckview.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSplitter>
#include <QTabWidget>
#include <QCloseEvent>
#include "card.h"
#include "carditem.h"
#include "cardslist.h"
#include "decklist.h"
#include "carddetails.h"
#include "remote.h"
#include "locallist.h"
#include "cardfilter.h"
#include "replaylist.h"
#include "scriptview.h"
#include "packview.h"
#include "pref.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void keyPressEvent(QKeyEvent *);

signals:
    void currentIdChanged(int);
private slots:
    void changeId(int);
    void toDetails(int);
    void save();
private:
    QTabWidget *tab;
    bool movedGlobal;
    QDialog *dialog;
    int currentId;
};

#endif // MAINWINDOW_H
