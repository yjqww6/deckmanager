#include "scriptview.h"
#include "expansions.h"
#include "configmanager.h"

ScriptView::ScriptView(QWidget *parent)
    : QDialog(parent)
{
    m_textEdit = new QPlainTextEdit;

    m_textEdit->setReadOnly(true);
    auto layout = new QVBoxLayout;
    layout->addWidget(m_textEdit, 1);
    m_textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setLayout(layout);
    resize(640, 480);
}

void ScriptView::setId(quint32 id)
{
    if(auto ocard = CardManager::inst().getCard(id))
    {
        Card &card = **ocard;
        setWindowTitle(card.name);
        QFile file("script/c" + QString::number(id) + ".lua");
        QString scriptText;
        m_textEdit->clear();
        if(file.open(QFile::ReadOnly))
        {
            scriptText = file.readAll();
        }
        else
        {
            scriptText = Expansions::inst().open("script/c" + QString::number(id) + ".lua");
        }

        m_textEdit->insertPlainText(scriptText);
        m_textEdit->setReadOnly(true);
        if(isHidden())
        {
            show();
        }
    }
}

static void printTo(QStringList &ls, Type::DeckI &deck, QString key, QString name, int mode)
{
    if(deck.size() > 0)
    {
        ls << ConfigManager::inst().getStr("label", key, name) + " " + QString:: number(deck.size());
        ls << "";
        foreach(auto &item, deck)
        {
            if(auto ocard = CardManager::inst().getCard(item.getId()))
            {
                Card &card = **ocard;
                if(mode == ScriptView::NORMAL)
                {
                    ls << card.name;
                }
                else
                {
                    ls << '[' + card.name + ']';
                }
            }
        }
    }
    ls << "";
}

static void printCount(QStringList &ls, Type::DeckI &deck, QString key, QString name)
{
    if(deck.size() > 0)
    {
        ls << ConfigManager::inst().getStr("label", key, name) + " " + QString:: number(deck.size());
        ls << "";
        QHash<quint32, int> map;
        foreach(auto &item, deck)
        {
            auto it = map.find(item.getId());
            if(it == map.end())
            {
                map.insert(item.getId(), 1);
            }
            else
            {
                map.insert(it.key(), it.value() + 1);
            }
        }
        auto list = map.keys();
        qSort(list.begin(), list.end(), idCompare);
        foreach(auto id, list)
        {
            if(auto ocard = CardManager::inst().getCard(id))
            {
                Card &card = **ocard;
                ls << QString::number(map.find(id).value()) + ' ' + card.name;
            }
        }
    }
    ls << "";
}

void ScriptView::setDeck(DeckModel *model, bool hideSide, int mode)
{
    m_textEdit->clear();
    QStringList ls;
    if(mode == NORMAL || mode == BRACKET)
    {
        printTo(ls, *model->m_mainDeck, "main", "主卡组", mode);
        printTo(ls, *model->m_extraDeck, "extra", "额外卡组", mode);
        if(!hideSide)
        {
            printTo(ls, *model->m_sideDeck, "side", "副卡组", mode);
        }
    }
    else if(mode == COUNT)
    {
        printCount(ls, *model->m_mainDeck, "main", "主卡组");
        printCount(ls, *model->m_extraDeck, "extra", "额外卡组");
        if(!hideSide)
        {
            printCount(ls, *model->m_sideDeck, "side", "副卡组");
        }
    }
    m_textEdit->insertPlainText(ls.join('\n'));
    m_textEdit->setReadOnly(false);
    auto cursor = m_textEdit->textCursor();
    cursor.setPosition(0);
    m_textEdit->setTextCursor(cursor);
    if(isHidden())
    {
        show();
    }
}
