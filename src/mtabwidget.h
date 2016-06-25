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
    QPair<int, int> currentIndex()
    {
        return m_current;
    }

signals:

public slots:
    void addTabBar();
    int addTab(int i, QString text);
    void addWidget(int tab, QWidget* widget, QString text);
    void setCurrentIndex(int tab, int column);
    void setCurrentIndex(QPair<int, int>);
    void changeSize();
private:
    void setCurrentIndexInner(int tab, int column);

    QList<QTabBar*> m_tabs;
    QPair<int, int> m_current;
    QStackedWidget  *m_stack;
    QVBoxLayout     *m_vbox;
};

#endif // MTABWIDGET_H
