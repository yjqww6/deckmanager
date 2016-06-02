#include "cardfilter.h"
#include "iconbutton.h"
#include "config.h"
#include "limitcards.h"
#include "range.h"
#include "wrapper.h"
#include "arrange.h"
#include "engine.h"
#include <QApplication>
#include <QCompleter>

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
    Const::TYPE_SYNCHRO | Const::TYPE_TUNER
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
    arrange arr("ab|cd|ee|"
                "f.|..|..|..|.f|gh|ij|kk|ll|mn|", grid);
    cardType = new QComboBox;
    cardType->setEditable(false);
    cardType->addItem("N/A", QVariant(-1));
    for(auto t : cardTypes)
    {
        cardType->addItem(cardPool->getType(t), t);
    }

    cardTypeSub = new QComboBox;
    cardTypeSub->setEditable(false);
    cardTypeSub->addItem("N/A", QVariant(-1));

    arr.set2('a', cardType, 'b', cardTypeSub);


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
    cardRace->addItem("N/A", QVariant(-1));
    cardAttr = new QComboBox;
    cardAttr->setEditable(false);
    cardAttr->addItem("N/A", QVariant(-1));

    passEdit = new QLineEdit;
    atkEdit = new QLineEdit;
    defEdit = new QLineEdit;
    levelEdit = new QLineEdit;
    rankEdit = new QLineEdit;
    scaleEdit = new QLineEdit;

    setEdit = new QComboBox;
    setEdit->setEditable(true);
    QStringList words;
    setEdit->addItem("");

    with_scheme([&]()
    {
       ptr v = engine->call("hashtable-values", Stop_level_value(Sstring_to_symbol("setnames")));
       if(Svectorp(v))
       {
           Slock_object(v);
           size_t len = Svector_length(v);
           for(size_t i = 0; i < len; ++i)
           {
               auto name = QString::fromUtf8(engine->getString(Svector_ref(v, i)).c_str());
               words << name;
               setEdit->addItem(name);
           }
           Sunlock_object(v);
       }
    });
    auto completer = new QCompleter(words, this);
    setEdit->setCompleter(completer);

    nameEdit = new QLineEdit;

    arr.set2('c', passL, 'd', passEdit);

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

    arr.set('e', hbox1);

    arrange arrM("ab|cd|ef|gh|ij|kl|mn", gridM);

    arrM.set2('a', raceL, 'b', cardRace);
    arrM.set2('c', attrL, 'd', cardAttr);
    arrM.set2('e', atkL, 'f', atkEdit);
    arrM.set2('g', defL, 'h', defEdit);
    arrM.set2('i', levelL, 'j', levelEdit);
    arrM.set2('k', rankL, 'l', rankEdit);
    arrM.set2('m', scaleL, 'n', scaleEdit);

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

    arr.set('f', tab);
    arr.set2('g', setL, 'h', setEdit);
    arr.set2('i', nameL, 'j', nameEdit);

    auto searchButton = new IconButton(":/icons/searchall.png", config->getStr("action", "searchall", "搜索"));
    auto searchThisButton = new IconButton(":/icons/search.png", config->getStr("action", "searchthis", "在结果中搜索"));
    auto searchDeckButton = new IconButton(":/icons/search.png", config->getStr("action", "searchdeck", "在卡组中搜索"));
    auto revertButton = new IconButton(":/icons/revert.png", config->getStr("action", "revert", "复位"));
    arr.set('k', searchButton);

    auto hbox = new QHBoxLayout;
    hbox->addWidget(revertButton);
    hbox->addWidget(searchDeckButton);
    hbox->addWidget(searchThisButton);

    arr.set('l', hbox);

    inverseMode = new QCheckBox;
    inverseMode->setText(config->getStr("label", "inverse", "反选模式"));

    noSortMode = new QCheckBox;
    noSortMode->setText(config->getStr("label", "nosort", "不排序"));

    arr.set2('m', inverseMode, 'n', noSortMode);
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
    cardTypeSub->addItem("N/A", ~0U);
    cardAttr->clear();
    cardAttr->addItem("N/A", ~0U);
    cardRace->clear();
    cardRace->addItem("N/A", ~0U);

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
    auto list = Type::DeckP::create();
    with_scheme([&]()
    {
        ptr res = engine->call("search-set", Sunsigned(id),
                               Sboolean(QApplication::keyboardModifiers() & Qt::ControlModifier));

        for(; res != Snil; res = Scdr(res))
        {
            list->append(Sunsigned_value(Scar(res)));
        }
    });

    if(!noSortMode->isChecked())
    {
        qSort(list->begin(), list->end(), idCompare);
    }

    emit result(list);
}

void CardFilter::searchAll()
{
    auto &map = cardPool->pool;
    search(keysBegin(map), keysEnd(map));
}

void CardFilter::searchThis()
{
    auto &t = getCurrent();
    search(t.begin(), t.end());
}

void CardFilter::searchDeck()
{
    auto deck = getDeck();
    search(deck->begin(), deck->end());
}

ptr CardFilter::make_pred()
{

    int type = cardType->currentData().toInt();
    int subtype = cardTypeSub->currentData().toInt();
    int race = cardRace->currentData().toInt();
    int attr = cardAttr->currentData().toInt();

    QString pass = passEdit->text();
    QString atk = atkEdit->text();
    QString def = defEdit->text();
    QString level = levelEdit->text();
    QString rank = rankEdit->text();
    QString scale = scaleEdit->text();

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
    int otC = ot->currentData().toInt();

    if(type == Const::TYPE_SPELL || type == Const::TYPE_TRAP)
    {
        subtype |= type;
    }

    QString setcode = setEdit->currentText();
    QString name = nameEdit->text();

    ptr param = Smake_vector(15, Sfalse);
    Slock_object(param);

    Svector_set(param, 0, Sfixnum(type));
    Svector_set(param, 1, Sfixnum(subtype));
    Svector_set(param, 2, Sfixnum(race));
    Svector_set(param, 3, Sfixnum(attr));
    Svector_set(param, 4, engine->fromQString(pass));
    Svector_set(param, 5, engine->fromQString(atk));
    Svector_set(param, 6, engine->fromQString(def));
    Svector_set(param, 7, engine->fromQString(level));
    Svector_set(param, 8, engine->fromQString(rank));
    Svector_set(param, 9, engine->fromQString(scale));
    Svector_set(param, 10, Sunsigned(category));
    Svector_set(param, 11, Sfixnum(limitC));
    Svector_set(param, 12, Sfixnum(otC));
    Svector_set(param, 13, engine->fromQString(setcode));
    Svector_set(param, 14, engine->fromQString(name));

    Sunlock_object(param);
    ptr ret = engine->call("card-filter", param);
    return ret;
}

template<typename T>
void CardFilter::search(T &&begin, T &&end)
{

    auto list = Type::DeckP::create();
    with_scheme([&]()
    {
        ptr pred = make_pred();
        Slock_object(pred);
        ptr ls = Snil;
        for(auto it = begin; it != end; ++it)
        {
            ls = Scons(Sunsigned(*it), ls);
        }
        ptr res = engine->call("search", ls, pred, Sboolean(inverseMode->isChecked()));

        Sunlock_object(pred);
        for(; res != Snil; res = Scdr(res))
        {
            list->append(Sunsigned_value(Scar(res)));
        }
    });

    if(!noSortMode->isChecked())
    {
        qSort(list->begin(), list->end(), idCompare);
    }

    emit result(list);

    return;
}

void CardFilter::revert()
{
    cardType->setCurrentIndex(0);
    cardTypeSub->setCurrentIndex(0);
    cardRace->setCurrentIndex(0);
    cardAttr->setCurrentIndex(0);
    setEdit->setCurrentIndex(0);


    passEdit->clear();
    atkEdit->clear();
    defEdit->clear();
    levelEdit->clear();
    rankEdit->clear();
    scaleEdit->clear();
    nameEdit->clear();

    for(auto effect : effects)
    {
        effect->setChecked(false);
    }
    inverseMode->setChecked(false);
}

CardFilter::~CardFilter()
{

}

