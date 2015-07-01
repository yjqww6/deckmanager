#ifndef MTABWIDGET_H
#define MTABWIDGET_H

#include <QWidget>
#include <QTabBar>
#include <QList>
#include <QStackedWidget>
#include <QVBoxLayout>

class MTabWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MTabWidget(QWidget *parent = 0);
    QList<QTabBar*> tabs;
    QPair<int, int> currentIndex()
    {
        return current;
    }

signals:

public slots:
    void addTabBar();
    int addTab(int i, QString text);
    void addWidget(int tab, QWidget* widget, QString text);
    void setCurrentIndex(int tab, int column);
    void setCurrentIndex(QPair<int, int>);
private:
    void setCurrentIndexInner(int tab, int column);
    QPair<int, int> current;
    QStackedWidget *stack;
    QVBoxLayout *vbox;
};

#endif // MTABWIDGET_H
