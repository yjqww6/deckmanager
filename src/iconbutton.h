#ifndef ICONBUTTON
#define ICONBUTTON

#include <QPushButton>

class IconButton : public QPushButton
{
public:
    IconButton(QString path, QString tooltip = "")
        : QPushButton()
    {
        setIcon(QIcon(path));
        setToolTip(tooltip);
    }
};

#endif // ICONBUTTON

