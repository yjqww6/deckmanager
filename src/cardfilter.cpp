#include "cardfilter.h"
#include "iconbutton.h"
#include "config.h"
#include "limitcards.h"
#include "range.h"

const quint32 CardFilter::cardTypes[] =
{
    Card::TYPE_MONSTER,
    Card::TYPE_SPELL,
    Card::TYPE_TRAP
};

const quint32 CardFilter::monsterTypes[] =
{
    Card::TYPE_NORMAL,
    Card::TYPE_EFFECT,
    Card::TYPE_FUSION,
    Card::TYPE_RITUAL,
    Card::TYPE_SPIRIT,
    Card::TYPE_UNION,
    Card::TYPE_DUAL,
    Card::TYPE_TUNER,
    Card::TYPE_SYNCHRO,
    Card::TYPE_FLIP,
    Card::TYPE_TOON,
    Card::TYPE_XYZ,
    Card::TYPE_PENDULUM,
    Card::TYPE_SYNCHRO | Card::TYPE_TUNER,
    Card::TYPE_NORMAL | Card::TYPE_TUNER
};

const quint32 CardFilter::spellTypes[] =
{
    Card::TYPE_QUICKPLAY,
    Card::TYPE_CONTINUOUS,
    Card::TYPE_EQUIP,
    Card::TYPE_FIELD,
    Card::TYPE_RITUAL
};

const quint32 CardFilter::trapTypes[] =
{
    Card::TYPE_CONTINUOUS,
    Card::TYPE_COUNTER
};

const quint32 CardFilter::monsterRaces[] =
{
    Card::RACE_WARRIOR,
    Card::RACE_SPELLCASTER,
    Card::RACE_FAIRY,
    Card::RACE_FIEND,
    Card::RACE_ZOMBIE,
    Card::RACE_MACHINE,
    Card::RACE_AQUA,
    Card::RACE_PYRO,
    Card::RACE_ROCK,
    Card::RACE_WINDBEAST,
    Card::RACE_PLANT,
    Card::RACE_INSECT,
    Card::RACE_THUNDER,
    Card::RACE_DRAGON,
    Card::RACE_BEAST,
    Card::RACE_BEASTWARRIOR,
    Card::RACE_DINOSAUR,
    Card::RACE_FISH,
    Card::RACE_SEASERPENT,
    Card::RACE_REPTILE,
    Card::RACE_PSYCHO,
    Card::RACE_DEVINE,
    Card::RACE_CREATORGOD,
    Card::RACE_PHANTOMDRAGON
};
const quint32 CardFilter::monsterAttrs[] =
{
    Card::ATTRIBUTE_DARK,
    Card::ATTRIBUTE_LIGHT,
    Card::ATTRIBUTE_WATER,
    Card::ATTRIBUTE_FIRE,
    Card::ATTRIBUTE_EARTH,
    Card::ATTRIBUTE_WIND,
    Card::ATTRIBUTE_DEVINE
};


CardFilter::CardFilter(QWidget *parent) : QWidget(parent)
{
    auto grid = new QGridLayout;
    cardType = new QComboBox;
    cardType->setEditable(false);
    cardType->addItem(tr("N/A"), QVariant(-1));
    for(auto t : cardTypes)
    {
        cardType->addItem(CardPool::getType(t), t);
    }

    cardTypeSub = new QComboBox;
    cardTypeSub->setEditable(false);
    cardTypeSub->addItem(tr("N/A"), QVariant(-1));


    int y = 0;

    grid->addWidget(cardType, y, 0);
    grid->addWidget(cardTypeSub, y, 1);

    connect(cardType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &CardFilter::setCardTypeSub);

    auto tab = new QTabWidget;
    auto gridM = new QGridLayout;

    auto passL = new QLabel(config->getStr("label", "pass", "卡密"));
    auto raceL = new QLabel(config->getStr("label", "race", "种族"));
    auto attrL = new QLabel(config->getStr("label", "attr", "属性"));
    auto atkL = new QLabel(config->getStr("label", "atk", "攻击"));
    auto defL = new QLabel(config->getStr("label", "def", "守备"));
    auto levelL = new QLabel(config->getStr("label", "level", "星/阶"));
    auto scaleL = new QLabel(config->getStr("label", "scale", "刻度"));


    auto setL = new QLabel(config->getStr("label", "setcode", "系列"));
    auto nameL = new QLabel(config->getStr("label", "keyword", "关键字"));

    cardRace = new QComboBox;
    cardRace->setEditable(false);
    cardRace->addItem(tr("N/A"), QVariant(-1));
    cardAttr = new QComboBox;
    cardAttr->setEditable(false);
    cardAttr->addItem(tr("N/A"), QVariant(-1));

    passEdit = new QLineEdit;
    atkEdit = new QLineEdit;
    defEdit = new QLineEdit;
    levelEdit = new QLineEdit;
    scaleEdit = new QLineEdit;


    setEdit = new QLineEdit;
    nameEdit = new QLineEdit;

    y++;
    grid->addWidget(passL, y, 0);
    grid->addWidget(passEdit, y, 1);

    y++;
    auto hbox1 = new QHBoxLayout;
    limit = new QComboBox;
    limit->addItem("N/A", -1);
    limit->addItem(config->getStr("label", "banned", "禁止"), 0);
    limit->addItem(config->getStr("label", "limited", "限制"), 1);
    limit->addItem(config->getStr("label", "semi-limited", "准限制"), 2);
    limit->addItem(config->getStr("label", "nolimited", "无限制"), 3);
    hbox1->addWidget(limit);

    ot = new QComboBox;
    ot->addItem("N/A", 0);
    ot->addItem("OCG", 1);
    ot->addItem("TCG", 2);
    hbox1->addWidget(ot);

    grid->addLayout(hbox1, y, 0, 1, 2);


    int y1 = y;
    y1++;
    gridM->addWidget(raceL, y1, 0);
    gridM->addWidget(cardRace, y1, 1);

    y1++;
    gridM->addWidget(attrL, y1, 0);
    gridM->addWidget(cardAttr, y1, 1);

    y1++;
    gridM->addWidget(atkL, y1, 0);
    gridM->addWidget(atkEdit, y1, 1);

    y1++;
    gridM->addWidget(defL, y1, 0);
    gridM->addWidget(defEdit, y1, 1);

    y1++;
    gridM->addWidget(levelL, y1, 0);
    gridM->addWidget(levelEdit, y1, 1);

    y1++;
    gridM->addWidget(scaleL, y1, 0);
    gridM->addWidget(scaleEdit, y1, 1);

    auto wM = new QWidget;
    wM->setLayout(gridM);
    tab->addTab(wM, config->getStr("label", "attribute", "属性"));

    auto gridE = new QGridLayout;
    for(int i : range(32))
    {
        effects[i] = new QCheckBox;
        effects[i]->setText(config->getStr("effect", QString::number(i), ""));
        gridE->addWidget(effects[i], i / 2, i % 2);
    }

    auto wE = new QScrollArea;
    wE->setLayout(gridE);
    tab->addTab(wE, config->getStr("label", "effect", "效果"));

    y++;
    grid->addWidget(tab, y, 0, 4, 2);

    y += 4;
    grid->addWidget(setL, y, 0);
    grid->addWidget(setEdit, y, 1);
    y++;
    grid->addWidget(nameL, y, 0);
    grid->addWidget(nameEdit, y, 1);

    auto searchButton = new IconButton(":/icons/searchall.png", config->getStr("action", "searchall", "搜索"));
    connect(searchButton, &QPushButton::clicked, this, &CardFilter::searchAll);

    auto searchThisButton = new IconButton(":/icons/search.png", config->getStr("action", "search", "在结果中搜索"));
    connect(searchThisButton, &QPushButton::clicked, this, &CardFilter::searchThis);

    auto revertButton = new IconButton(":/icons/revert.png", config->getStr("action", "revert", "复位"));
    connect(revertButton, &QPushButton::clicked, this, &CardFilter::revert);

    y++;
    grid->addWidget(searchButton, y, 0, 1, 2);

    auto hbox = new QHBoxLayout;

    hbox->addWidget(revertButton);
    hbox->addWidget(searchThisButton);

    y++;
    grid->addLayout(hbox, y, 0, 1, 2);
    setLayout(grid);
}

void CardFilter::setCardTypeSub(int index)
{
    cardTypeSub->clear();
    cardTypeSub->addItem(tr("N/A"), ~0U);
    cardAttr->clear();
    cardAttr->addItem(tr("N/A"), ~0U);
    cardRace->clear();
    cardRace->addItem(tr("N/A"), ~0U);

    switch(index)
    {
    case 0:
        break;
    case 1:
        for(auto t : monsterTypes)
        {
            cardTypeSub->addItem(CardPool::getType(t), QVariant(t));
        }
        for(auto t : monsterAttrs)
        {
            cardAttr->addItem(CardPool::getAttr(t), QVariant(t));
        }
        for(auto t : monsterRaces)
        {
            cardRace->addItem(CardPool::getRace(t), QVariant(t));
        }
        break;
    case 2:
        cardTypeSub->addItem(config->getStr("string", "TYPE_NORMAL", "通常"), 0);
        for(auto t : spellTypes)
        {
            cardTypeSub->addItem(CardPool::getType(t), QVariant(t));
        }
        break;
    case 3:
        cardTypeSub->addItem(config->getStr("string", "TYPE_NORMAL", "通常"), 0);
        for(auto t : trapTypes)
        {
            cardTypeSub->addItem(CardPool::getType(t), QVariant(t));
        }
        break;
    default:
        break;
    }
}

void CardFilter::searchAll()
{
    search(CardPool::getMap().keys());
}

void CardFilter::searchThis()
{
    auto &t = getCurrent();
    search(t);
}

template<typename T>

void CardFilter::search(const T &cards)
{
    auto ls = Type::DeckP::create();
    quint32 type = cardType->currentData().toUInt();
    quint32 subtype = cardTypeSub->currentData().toUInt();
    quint32 race = cardRace->currentData().toUInt();
    quint32 attr = cardAttr->currentData().toUInt();

    auto passRange = getRange(passEdit);
    auto atkRange = getRange(atkEdit);
    auto defRange = getRange(defEdit);
    auto levelRange = getRange(levelEdit);
    auto scaleRange = getRange(scaleEdit);

    quint32 category = 0;

    quint32 unit = 1;
    for(int i : range(32))
    {
        if(effects[i]->isChecked())
        {
            category |= unit;
        }
        unit = unit << 1;
    }

    int limitC = limit->currentData().toInt();
    quint32 otC = ot->currentData().toUInt();

    if(type == Card::TYPE_SPELL || type == Card::TYPE_TRAP)
    {
        subtype |= type;
    }

    quint64 set_code = setEdit->text().toULongLong(nullptr, 16);
    QString name = nameEdit->text();
    bool expr = false;
    QList<QStringList> exprs;
    if(name.indexOf('|') >= 0 || name.indexOf('&') >= 0)
    {
        expr = true;
        QStringList orExprs = name.split("|");
        foreach(auto &orExpr, orExprs)
        {
            exprs.append(orExpr.split("&"));
        }
    }

    getRange(atkEdit);
    ls->reserve(cards.size());

    foreach(auto it, cards)
    {
        auto card = CardPool::getCard(it);

        if(card->type & Card::TYPE_TOKEN)
        {
            continue;
        }

        if(type != ~0U && !(type & card->type))
        {
            continue;
        }

        if(limitC != -1 && LimitCards::getLimit(card->id) != limitC)
        {
            continue;
        }

        if(otC != 0 && (card->ot & 3) != otC)
        {
            continue;
        }

        if(!matchRange(passRange, card->id))
        {
            continue;
        }
        if(type & Card::TYPE_MONSTER)
        {
            if(subtype != ~0U && !((subtype & card->type) == subtype))
            {
                continue;
            }

            if(race != ~0U && !(race & card->race))
            {
                continue;
            }

            if(attr != ~0U && !(attr & card->attribute))
            {
                continue;
            }

            if(!matchRange(atkRange, card->atk))
            {
                continue;
            }

            if(!matchRange(defRange, card->def))
            {
                continue;
            }

            if(!matchRange(levelRange, card->level))
            {
                continue;
            }
            if(!matchRange(scaleRange, card->scale))
            {
                continue;
            }
        }
        else
        {
            if(subtype != ~0U && card->type != subtype)
            {
                continue;
            }
        }

        if((category & card->category) != category)
        {
            continue;
        }

        if(set_code)
        {
            quint64 setcode = card->setcode;
            quint64 settype = set_code & 0x0fff;
            quint64 setsubtype = set_code & 0xf000;
            bool found = false;
            while(setcode)
            {
                if((setcode & 0x0fff) == settype &&
                        (setsubtype == 0 || (setcode & 0xf000) == setsubtype))
                {
                    found = true;
                    break;
                }
                setcode = setcode >> 16;
            }
            if(!found)
            {
                continue;
            }
        }
        if(!name.isEmpty())
        {
            if(!expr)
            {
                if(card->name.indexOf(name) < 0 &&
                        card->effect.indexOf(name) < 0)
                {
                    continue;
                }
            }
            else
            {
                bool outer = false;
                foreach(auto &andExprs, exprs)
                {
                    bool inner = true;
                    foreach(auto &str, andExprs)
                    {
                        if(card->name.indexOf(str) < 0 &&
                                    card->effect.indexOf(str) < 0)
                        {
                            inner = false;
                            break;
                        }
                    }
                    if(inner)
                    {
                        outer = true;
                        break;
                    }
                }
                if(!outer)
                {
                    continue;
                }
            }
        }

        ls->append(card->id);
    }


    qSort(ls->begin(), ls->end(),
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
        else if((ca->type & Card::TYPE_MONSTER) && ca->atk != cb->atk)
        {
            return ca->atk >= cb->atk;
        }
        else
        {
            return a < b;
        }
    });

    emit result(ls);

    return;
}

bool CardFilter::matchRange(QPair<int, int> range, int num)
{
    if(range.first == -3)
    {
        return false;
    }

    if(range.first == -2)
    {
        return num == -2;
    }

    if(range.first != -1 && num < range.first)
    {
        return false;
    }

    if(range.second != -1 && num > range.second)
    {
        return false;
    }

    return true;
}

QPair<int, int> CardFilter::getRange(QLineEdit *lineEdit)
{
    QString line = lineEdit->text();

    enum
    {
        INIT,
        NUM,
        LE,
        LT,
        GE,
        GT,
        QUESTION,
        RANGE,
        ERR
    } state = INIT;

    int num1 = 0, num2 = 0;

    for(QChar ch : line)
    {
        unsigned int c = ch.unicode();
//        qDebug() << c;
        if(state == INIT)
        {
            if(c == '<')
            {
                num1 = -1;
                state = LT;
            }
            else if(c == '>')
            {
                num2 = -1;
                state = GT;
            }
            else if(c == '?')
            {
                num1 = -2;
                num2 = -2;
                state = QUESTION;
            }
            else if(isdigit(c))
            {
                state = NUM;
                num2 = -1;
                num1 = c - '0';
            }
            else
            {
                state = ERR;
                num1 = -3;
                num2 = -3;
                break;
            }
        }
        else if(state == LT)
        {
            if(c == '=')
            {
                state = LE;
            }
            else if(isdigit(c))
            {
                num2 = num2 * 10 + c - '0';
            }
            else
            {
                state = ERR;
                num1 = -3;
                num2 = -3;
                break;
            }
        }
        else if(state == GT)
        {
            if(c == '=')
            {
                state = GE;
            }
            else if(isdigit(c))
            {
                num1 = num1 * 10 + c - '0';
            }
            else
            {
                state = ERR;
                num1 = -3;
                num2 = -3;
                break;
            }
        }
        else if(state == LE)
        {
            if(isdigit(c))
            {
                num2 = num2 * 10 + c - '0';
            }
            else
            {
                state = ERR;
                num1 = -3;
                num2 = -3;
                break;
            }
        }
        else if(state == GE)
        {
            if(isdigit(c))
            {
                num1 = num1 * 10 + c - '0';
            }
            else
            {
                state = ERR;
                num1 = -3;
                num2 = -3;
                break;
            }
        }
        else if(state == NUM)
        {
            if(isdigit(c))
            {
                num1 = num1 * 10 + c - '0';
            }
            else if(c == '-')
            {
                num2 = 0;
                state = RANGE;
            }
            else
            {
                break;
            }
        }
        else if(state == QUESTION)
        {
            break;
        }
        else if(state == RANGE)
        {
            if(isdigit(c))
            {
                num2 = num2 * 10 + c - '0';
            }
            else
            {
                break;
            }
        }
        else if(state == ERR)
        {
            num1 = -3;
            num2 = -3;
            break;
        }
    }

    if(state == INIT)
    {
        num1 = -1;
        num2 = -1;
    }
    else if(state == NUM)
    {
        num2 = num1;
    }
    else if(state == LT)
    {
        num2 -= 1;
    }
    else if(state == GT)
    {
        num1 += 1;
    }

//    qDebug() << num1 << num2;
    return qMakePair(num1, num2);
}

void CardFilter::revert()
{
    cardType->setCurrentIndex(0);
    cardTypeSub->setCurrentIndex(0);
    cardRace->setCurrentIndex(0);
    cardAttr->setCurrentIndex(0);


    passEdit->clear();
    atkEdit->clear();
    defEdit->clear();
    levelEdit->clear();
    scaleEdit->clear();
    setEdit->clear();
    nameEdit->clear();

    for(auto effect : effects)
    {
        effect->setChecked(false);
    }
}

CardFilter::~CardFilter()
{

}

