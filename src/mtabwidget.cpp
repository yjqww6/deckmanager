#include "mtabwidget.h"
#include "range.h"
#include <QVariant>
#include <QDebug>
#include <QPalette>

MTabWidget::MTabWidget(QWidget *parent) : QWidget(parent)
{
    stack = new QStackedWidget;
    vbox = new QVBoxLayout;
    vbox->addWidget(stack, 1);
    vbox->setSpacing(0);
    vbox->setMargin(0);
    setLayout(vbox);
    stack->setStyleSheet("QStackedWidget{background: rgba(255, 255, 255, 200)}");
    current = qMakePair(-1, -1);
}

void MTabWidget::addTabBar()
{
    auto tab = new QTabBar(this);
    vbox->insertWidget(tabs.size(), tab);
    int tabIndex = tabs.size();
    tabs.append(tab);
    tabs.back()->setCurrentIndex(-1);
    tab->setExpanding(false);
    tab->setStyleSheet("QTabBar::tab:first{max-width:0px; border-width:0px}"
                       "QTabBar{background: rgba(255, 255, 255, 200)}");
    tab->setDrawBase(false);
    tab->addTab("");
    connect(tab, &QTabBar::tabBarClicked, [=](int index) {
        setCurrentIndexInner(tabIndex, index);
    });
}

void MTabWidget::setCurrentIndex(int tab, int column)
{
    setCurrentIndexInner(tab, column + 1);
}

void MTabWidget::setCurrentIndexInner(int tab, int column)
{
    if(column == -1)
    {
        return;
    }
    int s = tabs[tab]->tabData(column).toInt();
    stack->setCurrentIndex(s);
    current = qMakePair(tab, column - 1);
    for(int i: range(tabs.size()))
    {
        if(i == tab)
        {
            tabs[i]->setCurrentIndex(column);
        }
        else
        {
            tabs[i]->setCurrentIndex(0);
        }
    }
}

void MTabWidget::setCurrentIndex(QPair<int, int> rc)
{
    setCurrentIndex(rc.first, rc.second);
}

int MTabWidget::addTab(int i, QString text)
{
    if(i >= 0 && i < tabs.size())
    {
        return tabs[i]->addTab(text);
    }
    return -1;
}

void MTabWidget::addWidget(int tab, QWidget *widget, QString text)
{
    if(tab >= 0 && tab < tabs.size())
    {
        int column = addTab(tab, text);
        tabs[tab]->setTabData(column, stack->addWidget(widget));
    }
}
