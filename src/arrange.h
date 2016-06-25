#ifndef ARRANGE_H
#define ARRANGE_H

#include <string>
#include <QGridLayout>
#include <QWidget>
#include <utility>
#include <map>
class arrange
{
    struct info
    {
        qint8 x, y, w, h;
    };
public:
    arrange(const char* str, QGridLayout *grid) : m_layout(grid)
    {
        qint8 x = 0, y = 0;
        for(; *str; str++)
        {
            if(isalnum(*str))
            {
                auto it = m_infos.find(*str);
                if(it == m_infos.end())
                {
                    m_infos[*str] = info{x, y, 1, 1};
                }
                else
                {
                    info &i = it->second;
                    i.w = x - i.x + 1;
                    i.h = y - i.y + 1;
                }
                x++;
            }
            else if(*str == '|')
            {
                y++;
                x = 0;
            }
            else
            {
                x++;
            }

        }
    }

    void set(char c, QWidget *widget)
    {
        auto it = m_infos.find(c);
        if(it != m_infos.end())
        {
            info &i = it->second;
            m_layout->addWidget(widget, i.y, i.x, i.h, i.w);
        }
    }

    void set(char c, QLayout *widget)
    {
        auto it = m_infos.find(c);
        if(it != m_infos.end())
        {
            info &i = it->second;
            m_layout->addLayout(widget, i.y, i.x, i.h, i.w);
        }
    }


    void set2()
    {

    }

    template <typename T, typename... Args>
    void set2(char c, T* thing, Args... args)
    {
        set(c, thing);
        set2(args...);
    }

private:
    std::map<char, info> m_infos;
    QGridLayout *m_layout;
};

#endif // ARRANGE_H
