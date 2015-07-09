#include "cardfilter.h"
#include "iconbutton.h"
#include "config.h"
#include "limitcards.h"
#include "range.h"
#include "wrapper.h"

const quint32 CardFilter::cardTypes[] =
{
    Const::TYPE_MONSTER,
    Const::TYPE_SPELL,
    Const::TYPE_TRAP
};

const quint32 CardFilter::monsterTypes[] =
{
    Const::TYPE_NORMAL,
    Const::TYPE_EFFECT,
    Const::TYPE_FUSION,
    Const::TYPE_RITUAL,
    Const::TYPE_SPIRIT,
    Const::TYPE_UNION,
    Const::TYPE_DUAL,
    Const::TYPE_TUNER,
    Const::TYPE_SYNCHRO,
    Const::TYPE_FLIP,
    Const::TYPE_TOON,
    Const::TYPE_XYZ,
    Const::TYPE_PENDULUM,
    Const::TYPE_SYNCHRO | Const::TYPE_TUNER,
    Const::TYPE_NORMAL | Const::TYPE_TUNER
};

const quint32 CardFilter::spellTypes[] =
{
    Const::TYPE_QUICKPLAY,
    Const::TYPE_CONTINUOUS,
    Const::TYPE_EQUIP,
    Const::TYPE_FIELD,
    Const::TYPE_RITUAL
};

const quint32 CardFilter::trapTypes[] =
{
    Const::TYPE_CONTINUOUS,
    Const::TYPE_COUNTER
};

const quint32 CardFilter::monsterRaces[] =
{
    Const::RACE_WARRIOR,
    Const::RACE_SPELLCASTER,
    Const::RACE_FAIRY,
    Const::RACE_FIEND,
    Const::RACE_ZOMBIE,
    Const::RACE_MACHINE,
    Const::RACE_AQUA,
    Const::RACE_PYRO,
    Const::RACE_ROCK,
    Const::RACE_WINDBEAST,
    Const::RACE_PLANT,
    Const::RACE_INSECT,
    Const::RACE_THUNDER,
    Const::RACE_DRAGON,
    Const::RACE_BEAST,
    Const::RACE_BEASTWARRIOR,
    Const::RACE_DINOSAUR,
    Const::RACE_FISH,
    Const::RACE_SEASERPENT,
    Const::RACE_REPTILE,
    Const::RACE_PSYCHO,
    Const::RACE_DEVINE,
    Const::RACE_CREATORGOD,
    Const::RACE_PHANTOMDRAGON
};
const quint32 CardFilter::monsterAttrs[] =
{
    Const::ATTRIBUTE_DARK,
    Const::ATTRIBUTE_LIGHT,
    Const::ATTRIBUTE_WATER,
    Const::ATTRIBUTE_FIRE,
    Const::ATTRIBUTE_EARTH,
    Const::ATTRIBUTE_WIND,
    Const::ATTRIBUTE_DEVINE
};


CardFilter::CardFilter(QWidget *parent) : QWidget(parent)
{
    auto grid = new QGridLayout;
    cardType = new QComboBox;
    cardType->setEditable(false);
    cardType->addItem(tr("N/A"), QVariant(-1));
    for(auto t : cardTypes)
    {
        cardType->addItem(cardPool->getType(t), t);
    }

    cardTypeSub = new QComboBox;
    cardTypeSub->setEditable(false);
    cardTypeSub->addItem(tr("N/A"), QVariant(-1));


    int y = 0;

    grid->addWidget(cardType, y, 0);
    grid->addWidget(cardTypeSub, y, 1);


    auto tab = new QTabWidget;
    auto gridM = new QGridLayout;

    auto passL = new QLabel(config->getStr("label", "pass", "卡密"));
    auto raceL = new QLabel(config->getStr("label", "race", "种族"));
    auto attrL = new QLabel(config->getStr("label", "attr", "属性"));
    auto atkL = new QLabel(config->getStr("label", "atk", "攻击"));
    auto defL = new QLabel(config->getStr("label", "def", "守备"));
    auto levelL = new QLabel(config->getStr("label", "level", "等级"));
    auto rankL = new QLabel(config->getStr("label", "rank", "阶级"));
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
    rankEdit = new QLineEdit;
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
    gridM->addWidget(rankL, y1, 0);
    gridM->addWidget(rankEdit, y1, 1);

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
    auto searchThisButton = new IconButton(":/icons/search.png", config->getStr("action", "searchthis", "在结果中搜索"));
    auto searchDeckButton = new IconButton(":/icons/search.png", config->getStr("action", "searchdeck", "在卡组中搜索"));
    auto revertButton = new IconButton(":/icons/revert.png", config->getStr("action", "revert", "复位"));
    y++;
    grid->addWidget(searchButton, y, 0, 1, 2);

    auto hbox = new QHBoxLayout;
    hbox->addWidget(revertButton);
    hbox->addWidget(searchDeckButton);
    hbox->addWidget(searchThisButton);

    y++;
    grid->addLayout(hbox, y, 0, 1, 2);
    setLayout(grid);

    connect(cardType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &CardFilter::setCardTypeSub);
    connect(searchButton, &QPushButton::clicked, this, &CardFilter::searchAll);
    connect(searchDeckButton, &QPushButton::clicked, this, &CardFilter::searchDeck);
    connect(searchThisButton, &QPushButton::clicked, this, &CardFilter::searchThis);
    connect(revertButton, &QPushButton::clicked, this, &CardFilter::revert);
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
            cardTypeSub->addItem(cardPool->getType(t), QVariant(t));
        }
        for(auto t : monsterAttrs)
        {
            cardAttr->addItem(cardPool->getAttr(t), QVariant(t));
        }
        for(auto t : monsterRaces)
        {
            cardRace->addItem(cardPool->getRace(t), QVariant(t));
        }
        break;
    case 2:
        cardTypeSub->addItem(config->getStr("string", "TYPE_NORMAL", "通常"), 0);
        for(auto t : spellTypes)
        {
            cardTypeSub->addItem(cardPool->getType(t), QVariant(t));
        }
        break;
    case 3:
        cardTypeSub->addItem(config->getStr("string", "TYPE_NORMAL", "通常"), 0);
        for(auto t : trapTypes)
        {
            cardTypeSub->addItem(cardPool->getType(t), QVariant(t));
        }
        break;
    default:
        break;
    }
}

void CardFilter::searchSet(quint32 id)
{
    call_with_ref([&](Card &card) {
        quint64 setcode1 = card.setcode;

        auto &map = cardPool->getMap();
        auto ls = Type::DeckP::create();
        for(auto it = map.begin(); it != map.end(); ++it)
        {
            quint64 set_code = setcode1;
            bool foundO = false;

            if(it.value()->type & Const::TYPE_TOKEN)
            {
                continue;
            }

            while(set_code)
            {
                quint64 setcode2 = it.value()->setcode;
                quint64 settype = set_code & 0x0fff;
                bool found = false;
                while(setcode2)
                {
                    if((setcode2 & 0x0fff) == settype)
                    {
                        found = true;
                        break;
                    }
                    setcode2 = setcode2 >> 16;
                }

                if(found)
                {
                    foundO = true;
                    break;
                }
                set_code = set_code >> 16;
            }
            if(foundO)
            {
                ls->append(it.value()->id);
            }
        }
        if(!ls->empty())
        {
            qSort(ls->begin(), ls->end(), idCompare);
            emit result(ls);
        }
    }, cardPool->getCard(id));
}

void CardFilter::searchAll()
{
    search(cardPool->getMap().keys());
}

void CardFilter::searchThis()
{
    auto &t = getCurrent();
    search(t);
}

void CardFilter::searchDeck()
{
    auto deck = getDeck();
    search(*deck.data());
}

static QPair<int, int> ignore(-1, -1);

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
    auto rankRange = getRange(rankEdit);
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

    if(type == Const::TYPE_SPELL || type == Const::TYPE_TRAP)
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
        call_with_ref([&](Card& card) {

            if(card.type & Const::TYPE_TOKEN)
            {
                return;
            }

            if(type != ~0U && !(type & card.type))
            {
                return;
            }

            if(limitC != -1 && limitCards->getLimit(card.id) != limitC)
            {
                return;
            }

            if(otC != 0 && (card.ot & 3) != otC)
            {
                return;
            }

            if(!matchRange(passRange, card.id))
            {
                return;
            }
            if(type & Const::TYPE_MONSTER)
            {
                if(subtype != ~0U && !((subtype & card.type) == subtype))
                {
                    return;
                }

                if(race != ~0U && !(race & card.race))
                {
                    return;
                }

                if(attr != ~0U && !(attr & card.attribute))
                {
                    return;
                }

                if(!matchRange(atkRange, card.atk))
                {
                    return;
                }

                if(!matchRange(defRange, card.def))
                {
                    return;
                }

                if(levelRange != ignore)
                {
                    if((card.type & Const::TYPE_XYZ) || !matchRange(levelRange, card.level))
                    {
                        return;
                    }
                }

                if(rankRange != ignore)
                {
                    if(!(card.type & Const::TYPE_XYZ) || !matchRange(rankRange, card.level))
                    {
                        return;
                    }
                }

                if(scaleRange != ignore)
                {
                    if(!(card.type & Const::TYPE_PENDULUM) || !matchRange(scaleRange, card.scale))
                    {
                        return;
                    }
                }
            }
            else
            {
                if(subtype != ~0U && card.type != subtype)
                {
                    return;
                }
            }

            if((category & card.category) != category)
            {
                return;
            }

            if(set_code)
            {
                quint64 setcode = card.setcode;
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
                    return;
                }
            }
            if(!name.isEmpty())
            {
                if(!expr)
                {
                    if(card.name.indexOf(name) < 0 &&
                            card.effect.indexOf(name) < 0)
                    {
                        return;
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
                            if(card.name.indexOf(str) < 0 &&
                                    card.effect.indexOf(str) < 0)
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
                        return;
                    }
                }
            }

            ls->append(card.id);
        }, cardPool->getCard(it));
    }


    qSort(ls->begin(), ls->end(), idCompare);

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

