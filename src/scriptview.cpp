#include "scriptview.h"
#include "expansions.h"
#include "config.h"

ScriptView::ScriptView(QWidget *parent)
    : QDialog(parent)
{
    textEdit = new QPlainTextEdit;

    textEdit->setReadOnly(true);
    auto layout = new QVBoxLayout;
    layout->addWidget(textEdit, 1);
    textEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setLayout(layout);
    resize(640, 480);
}

void ScriptView::setId(quint32 id)
{
    call_with_ref([&](Card &card) {
        setWindowTitle(card.name);
        QFile file("script/c" + QString::number(id) + ".lua");
        QString scriptText;
        textEdit->clear();
        if(file.open(QFile::ReadOnly))
        {
            scriptText = file.readAll();
        }
        else
        {
            scriptText = expansions->open("script/c" + QString::number(id) + ".lua");
        }

        textEdit->insertPlainText(scriptText);
        textEdit->setReadOnly(true);
        if(isHidden())
        {
            show();
        }
    }, cardPool->getCard(id));
}

static void printTo(QStringList &ls, Type::DeckI &deck, QString key, QString name, int mode)
{
    if(deck.size() > 0)
    {
        ls << config->getStr("label", key, name) + " " + QString:: number(deck.size());
        ls << "";
        foreach(auto &item, deck)
        {
            call_with_ref([&](Card &card) {
                if(mode == ScriptView::NORMAL)
                {
                    ls << card.name;
                }
                else
                {
                    ls << '[' + card.name + ']';
                }
            }, cardPool->getCard(item.getId()));
        }
    }
    ls << "";
}

static void printCount(QStringList &ls, Type::DeckI &deck, QString key, QString name)
{
    if(deck.size() > 0)
    {
        ls << config->getStr("label", key, name) + " " + QString:: number(deck.size());
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
            call_with_ref([&](Card &card) {
                ls << QString::number(map.find(id).value()) + ' ' + card.name;
            }, cardPool->getCard(id));
        }
    }
    ls << "";
}

void ScriptView::setDeck(DeckModel *model, bool hideSide, int mode)
{
    textEdit->clear();
    QStringList ls;
    if(mode == NORMAL || mode == BRACKET)
    {
        printTo(ls, *model->mainDeck, "main", "主卡组", mode);
        printTo(ls, *model->extraDeck, "extra", "额外卡组", mode);
        if(!hideSide)
        {
            printTo(ls, *model->sideDeck, "side", "副卡组", mode);
        }
    }
    else if(mode == COUNT)
    {
        printCount(ls, *model->mainDeck, "main", "主卡组");
        printCount(ls, *model->extraDeck, "extra", "额外卡组");
        if(!hideSide)
        {
            printCount(ls, *model->sideDeck, "side", "副卡组");
        }
    }
    textEdit->insertPlainText(ls.join('\n'));
    textEdit->setReadOnly(false);
    auto cursor = textEdit->textCursor();
    cursor.setPosition(0);
    textEdit->setTextCursor(cursor);
    if(isHidden())
    {
        show();
    }
}
