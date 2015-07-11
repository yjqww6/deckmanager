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
    void currentIdChanged(quint32);
private slots:
    void changeId(quint32);
    void toDetails(quint32);
private:
    MTabWidget *tab;
    ScriptView *dialog;
    quint32 currentId;
};

#endif // MAINWINDOW_H
