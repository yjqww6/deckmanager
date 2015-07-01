#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTabWidget>
#include "mtabwidget.h"
#include "scriptview.h"

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
    MTabWidget *tab;
    bool movedGlobal;
    ScriptView *dialog;
    int currentId;
};

#endif // MAINWINDOW_H
