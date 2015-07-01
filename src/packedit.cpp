#include "packedit.h"
#include "iconbutton.h"
#include "draghelper.h"

PackEdit::PackEdit(QWidget *parent)
    : CardsList(parent)
{

}

int PackEdit::itemAt2(const QPoint _pos)
{
    for(int i = cardsPerColumn - 1; i >= 0; i--)
    {
        if(i + pos >= ls.size())
        {
            continue;
        }
        int index = i +  pos;

        auto &item = items.find(index).value();

        if(_pos.y() >= item.getPos().y())
        {
            return index;
        }
    }
    return -1;
}

int PackEdit::posIndex(QPoint point)
{
    int index = itemAt2(point);
    if(index == -1)
    {
        return -1;
    }
    auto it = items.find(index);
    int y = it->getPos().y();

    if(point.y() > y + cardSize.height() / 2)
    {
        index += 1;
    }

    return index;
}

bool PackEdit::filter(int id)
{
    auto it = ls.begin();
    for(; it != ls.end(); ++it)
    {
        if(*it == id)
        {
            break;
        }
    }

    if(it == ls.end())
    {
        return true;
    }
    else
    {
        return false;
    }
}

void PackEdit::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::RightButton)
    {
        int index = itemAt(event->pos());
        ls.removeAt(index);
        update();
    }
    CardsList::mouseMoveEvent(event);
}

void PackEdit::startDrag(int index)
{
    if(index >= ls.size())
    {
        return;
    }
    QMimeData *mimedata = new QMimeData;
    int id = ls[index];
    mimedata->setText(QString::number(id));
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimedata);
    auto &item = items.find(index).value();
    drag->setPixmap(item.getPixmap()->scaled(cardSize));
    drag->setHotSpot(QPoint(drag->pixmap().width() / 2,
                            drag->pixmap().height() / 2));

    bool copy = true;
    if((QApplication::keyboardModifiers() & Qt::ControlModifier) == 0)
    {
        ls.removeAt(index);
        copy = false;
    }
    current = -1;
    update();
    dragHelper.moved = false;
    drag->exec(Qt::MoveAction);
    if(!dragHelper.moved && !copy)
    {
        ls.append(id);
    }
    update();
    emit sizeChanged(ls.size());
}

void PackEdit::dragEnterEvent(QDragEnterEvent *event)
{
    auto src = event->source();

    if(src)
    {
        int id = event->mimeData()->text().toInt();
        if(filter(id))
        {
          event->accept();
        }
        else
        {
            event->ignore();
        }
    }
}

void PackEdit::dropEvent(QDropEvent *event)
{
    auto src = event->source();
    if(src)
    {
        int id = event->mimeData()->text().toInt();
        if(filter(id))
        {
            int index = posIndex(event->pos());
            if(index == -1)
            {
                ls.append(id);
            }
            else
            {
                ls.insert(index, id);
            }
            dragHelper.moved = true;
            update();
        }
        event->accept();
    }
    emit sizeChanged(ls.size());
}

void PackEdit::saveList(QString name)
{
    QDir dir;
    dir.mkpath("pack/" + config->getStr("pack", "mypackpath", "__我的卡包") + "/");
    QFile file("pack/" + config->getStr("pack", "mypackpath", "__我的卡包") + "/" + name + ".ypk");
    if(file.open(QFile::WriteOnly | QFile::Text))
    {
        QTextStream out(&file);
        foreach(int id, ls)
        {
            out << id << '\n';
        }
        emit saved();
    }
}

PackEditView::PackEditView(QWidget *parent)
    : QWidget(parent)
{

    pe = new PackEdit(nullptr);
    auto hbox = new QHBoxLayout;
    auto vbox = new QVBoxLayout;
    hbox->addWidget(pe);
    connect(pe, &PackEdit::currentIdChanged, this, &PackEditView::changeId);
    auto sb = new QScrollBar;
    hbox->addWidget(sb);

    auto label = new DeckSizeLabel(config->getStr("label", "number", "数目"));
    label->setAlignment(Qt::AlignRight);
    label->changeSize(0);
    vbox->addWidget(label);

    pe->setScrollBar(sb);

    vbox->addLayout(hbox, 1);

    nameEdit = new QLineEdit;

    hbox = new QHBoxLayout;


    auto saveButton = new IconButton(":/icons/saveas.png", config->getStr("action", "saveas", "另存为"));
    auto sortButton = new IconButton(":/icons/sort.png", config->getStr("action", "sort", "排序"));
    auto clearButton = new IconButton(":/icons/clear.png", config->getStr("action", "clear", "清空"));

    hbox->addWidget(nameEdit);
    hbox->addWidget(saveButton);

    vbox->addLayout(hbox);

    hbox = new QHBoxLayout;
    hbox->addWidget(sortButton);
    hbox->addWidget(clearButton);
    vbox->addLayout(hbox);

    connect(pe, &PackEdit::sizeChanged, label, &DeckSizeLabel::changeSize);
    connect(pe, &PackEdit::clickId, this, &PackEditView::idClicked);
    connect(saveButton, &IconButton::clicked, this, &PackEditView::saveList);
    connect(clearButton, &IconButton::clicked, pe, &PackEdit::clearList);
    connect(sortButton, &IconButton::clicked, pe, &PackEdit::sort);
    connect(pe, &PackEdit::saved, this, &PackEditView::saved);
    connect(pe, &PackEdit::details, this, &PackEditView::details);
    setLayout(vbox);
}
void PackEdit::sort()
{
    qSort(ls.begin(), ls.end(),
          [&](int a, int b)
    {
        auto ca = CardPool::getCard(a);
        auto cb = CardPool::getCard(b);
        int ta = ca->type & 7, tb = cb->type & 7;
        if(ta != tb)
        {
            return ta < tb;
        }
        else if(ca->type != cb->type)
        {
            return ca->type < cb->type;
        }
        else if(ca->type & Card::TYPE_MONSTER)
        {
            return ca->atk >= cb->atk;
        }
        else
        {
            return a < b;
        }
    });
    update();
}
