#include "packedit.h"
#include "iconbutton.h"
#include "draghelper.h"

PackEdit::PackEdit(QWidget *parent)
    : CardsList(parent)
{
    setMinimumWidth(200);
}

int PackEdit::itemAt2(const QPoint _pos)
{
    for(int i = m_cardsPerColumn - 1; i >= 0; i--)
    {
        if(i + m_currentPos >= m_deck.size())
        {
            continue;
        }
        int index = i +  m_currentPos;

        auto &item = m_items.find(index).value();

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
    auto it = m_items.find(index);
    int y = it->getPos().y();

    if(point.y() > y + m_cardSize.height() / 2)
    {
        index += 1;
    }

    return index;
}

bool PackEdit::filter(quint32 id)
{
    foreach(auto it, m_deck)
    {
        if(it == id)
        {
            return false;
        }
    }
    return true;
}

void PackEdit::mousePressEvent(QMouseEvent *event)
{
    if(event->buttons() & Qt::RightButton)
    {
        int index = itemAt(event->pos());
        m_deck.removeAt(index);
        update();
    }
    CardsList::mousePressEvent(event);
}

void PackEdit::startDrag(int index)
{
    if(index >= m_deck.size())
    {
        return;
    }
    QMimeData *mimedata = new QMimeData;
    quint32 id = m_deck[index];
    mimedata->setText(QString::number(id));
    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimedata);
    auto &item = m_items.find(index).value();
    if(item.getPixmap())
    {
        drag->setPixmap(item.getPixmap()->scaled(m_cardSize));
        drag->setHotSpot(QPoint(drag->pixmap().width() / 2,
                                drag->pixmap().height() / 2));
    }

    bool copy = true;
    if((QApplication::keyboardModifiers() & Qt::ControlModifier) == 0)
    {
        m_deck.removeAt(index);
        copy = false;
    }
    m_current = -1;
    update();
    DragHelper::inst().moved = false;
    drag->exec(Qt::MoveAction);
    if(!DragHelper::inst().moved && !copy)
    {
        m_deck.append(id);
    }
    update();
    emit sizeChanged(m_deck.size());
}

void PackEdit::dragEnterEvent(QDragEnterEvent *event)
{
    auto src = event->source();

    if(src)
    {
        quint32 id = event->mimeData()->text().toUInt();
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
        quint32 id = event->mimeData()->text().toUInt();
        if(filter(id))
        {
            int index = posIndex(event->pos());
            if(index == -1)
            {
                m_deck.append(id);
            }
            else
            {
                m_deck.insert(index, id);
            }
            DragHelper::inst().moved = true;
            update();
        }
        event->accept();
    }
    emit sizeChanged(m_deck.size());
}

void PackEdit::saveList(QString name)
{
    QDir dir;
    dir.mkpath("pack/" + ConfigManager::inst().getStr("pack", "mypackpath", "__我的卡包") + "/");
    QFile file("pack/" + ConfigManager::inst().getStr("pack", "mypackpath", "__我的卡包") + "/" + name + ".ypk");
    if(file.open(QFile::WriteOnly | QFile::Text))
    {
        QTextStream out(&file);
        foreach(auto id, m_deck)
        {
            out << id << '\n';
        }
        emit saved();
    }
}

PackEditView::PackEditView(QWidget *parent)
    : QWidget(parent)
{

    m_pe = new PackEdit(nullptr);
    auto sb = new QScrollBar;
    m_pe->setScrollBar(sb);

    auto label = new DeckSizeLabel(ConfigManager::inst().getStr("label", "number", "数目"));
    label->setAlignment(Qt::AlignRight);
    label->changeSize(0);

    m_nameEdit = new QLineEdit;

    auto saveButton = new IconButton(":/icons/saveas.png", ConfigManager::inst().getStr("action", "saveas", "另存为"));
    auto sortButton = new IconButton(":/icons/sort.png", ConfigManager::inst().getStr("action", "sort", "排序"));
    auto clearButton = new IconButton(":/icons/clear.png", ConfigManager::inst().getStr("action", "clear", "清空"));

    connect(m_pe, &PackEdit::sizeChanged, label, &DeckSizeLabel::changeSize);
    connect(saveButton, &IconButton::clicked, this, &PackEditView::saveList);
    connect(clearButton, &IconButton::clicked, m_pe, &PackEdit::clearList);
    connect(sortButton, &IconButton::clicked, m_pe, &PackEdit::sort);
    connect(m_pe, &PackEdit::saved, this, &PackEditView::saved);

    auto hbox = new QHBoxLayout;
    auto vbox = new QVBoxLayout;
    hbox->addWidget(m_pe);
    hbox->addWidget(sb);
    vbox->addWidget(label);
    vbox->addLayout(hbox, 1);
    hbox = new QHBoxLayout;
    hbox->addWidget(m_nameEdit);
    hbox->addWidget(saveButton);
    vbox->addLayout(hbox);
    hbox = new QHBoxLayout;
    hbox->addWidget(sortButton);
    hbox->addWidget(clearButton);
    vbox->addLayout(hbox);

    setLayout(vbox);
    label->changeSize(0);
}
void PackEdit::sort()
{
    qSort(m_deck.begin(), m_deck.end(), idCompare);
    update();
}
