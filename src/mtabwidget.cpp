#include "mtabwidget.h"
#include "range.h"
#include <QVariant>
#include <QDebug>
#include <QPalette>

MTabWidget::MTabWidget(QWidget *parent) : QWidget(parent)
{
    m_stack = new QStackedWidget;
    m_vbox = new QVBoxLayout;
    m_vbox->addWidget(m_stack, 1);
    m_vbox->setSpacing(0);
    m_vbox->setMargin(0);
    setLayout(m_vbox);
    m_stack->setStyleSheet("QStackedWidget{background: rgba(255, 255, 255, 200)}");
    m_current = qMakePair(-1, -1);
}

void MTabWidget::addTabBar()
{
    auto tab = new QTabBar(this);
    m_vbox->insertWidget(m_tabs.size(), tab);
    int tabIndex = m_tabs.size();
    m_tabs.append(tab);
    m_tabs.back()->setCurrentIndex(-1);
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
    int s = m_tabs[tab]->tabData(column).toInt();
    m_stack->setCurrentIndex(s);
    m_current = qMakePair(tab, column - 1);
    for(int i: range(m_tabs.size()))
    {
        if(i == tab)
        {
            m_tabs[i]->setCurrentIndex(column);
        }
        else
        {
            m_tabs[i]->setCurrentIndex(0);
        }
    }
}

void MTabWidget::setCurrentIndex(QPair<int, int> rc)
{
    setCurrentIndex(rc.first, rc.second);
}

int MTabWidget::addTab(int i, QString text)
{
    if(i >= 0 && i < m_tabs.size())
    {
        return m_tabs[i]->addTab(text);
    }
    return -1;
}

void MTabWidget::changeSize()
{
    int max = 220;
    foreach(auto tabbar, m_tabs)
    {
        int cur = 0;
        for(int i : range(tabbar->count()))
        {
            cur += tabbar->tabRect(i).width();
        }
        if(cur > max)
        {
            max = cur;
        }
    }
    setMinimumWidth(max);
}

void MTabWidget::addWidget(int tab, QWidget *widget, QString text)
{
    if(tab >= 0 && tab < m_tabs.size())
    {
        int column = addTab(tab, text);
        m_tabs[tab]->setTabData(column, m_stack->addWidget(widget));
    }
}
