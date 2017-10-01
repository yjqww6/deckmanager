#include "cardfilter.h"
#include "iconbutton.h"
#include "configmanager.h"
#include "limitcards.h"
#include "range.h"
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
    Const::TYPE_SPSUMMON,
    Const::TYPE_LINK,
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
    Const::RACE_WYRM,
    Const::RACE_CYBERS
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
    m_cardType = new QComboBox;
    m_cardType->setEditable(false);
    m_cardType->addItem("N/A", QVariant(-1));
    for(auto t : cardTypes)
    {
        m_cardType->addItem(CardManager::inst().getType(t), t);
    }

    m_cardTypeSub = new QComboBox;
    m_cardTypeSub->setEditable(false);
    m_cardTypeSub->addItem("N/A", QVariant(-1));

    arr.set2('a', m_cardType, 'b', m_cardTypeSub);


    auto tab = new QTabWidget;
    auto gridM = new QGridLayout;

    auto passL = new QLabel(ConfigManager::inst().getStr("label", "pass", "卡密"));
    auto raceL = new QLabel(ConfigManager::inst().getStr("label", "race", "种族"));
    auto attrL = new QLabel(ConfigManager::inst().getStr("label", "attr", "属性"));
    auto atkL = new QLabel(ConfigManager::inst().getStr("label", "atk", "攻击"));
    auto defL = new QLabel(ConfigManager::inst().getStr("label", "def", "守备"));
    auto levelL = new QLabel(ConfigManager::inst().getStr("label", "level", "等级"));
    auto rankL = new QLabel(ConfigManager::inst().getStr("label", "rank", "阶级"));
    auto scaleL = new QLabel(ConfigManager::inst().getStr("label", "scale", "刻度"));
    auto linkL = new QLabel(ConfigManager::inst().getStr("label", "link", "LINK"));


    auto setL = new QLabel(ConfigManager::inst().getStr("label", "setcode", "系列"));
    auto nameL = new QLabel(ConfigManager::inst().getStr("label", "keyword", "关键字"));

    m_cardRace = new QComboBox;
    m_cardRace->setEditable(false);
    m_cardRace->addItem("N/A", QVariant(-1));
    m_cardAttr = new QComboBox;
    m_cardAttr->setEditable(false);
    m_cardAttr->addItem("N/A", QVariant(-1));

    m_passEdit = new QLineEdit;
    m_atkEdit = new QLineEdit;
    m_defEdit = new QLineEdit;
    m_levelEdit = new QLineEdit;
    m_rankEdit = new QLineEdit;
    m_scaleEdit = new QLineEdit;
    m_linkEdit = new QLineEdit;

    m_setEdit = new QComboBox;
    m_setEdit->setEditable(true);
    QStringList words;
    m_setEdit->addItem("");

    with_scheme([&]()
    {
        ptr table = Stop_level_value(Sstring_to_symbol("setnames"));
        Slock_object(table);
        if(engine->call("hashtable?", table) != Sfalse)
        {
            ptr v = engine->call("hashtable-values", table);
            if(Svectorp(v))
            {
                size_t len = Svector_length(v);
                for(size_t i = 0; i < len; ++i)
                {
                    auto name = engine->getString(Svector_ref(v, i));
                    words << name;
                    m_setEdit->addItem(name);
                }
            }
        }
        Sunlock_object(table);
    });
    auto completer = new QCompleter(words, this);
    m_setEdit->setCompleter(completer);

    m_nameEdit = new QLineEdit;

    arr.set2('c', passL, 'd', m_passEdit);

    auto hbox1 = new QHBoxLayout;
    m_limit = new QComboBox;
    m_limit->addItem("N/A", -1);
    m_limit->addItem(ConfigManager::inst().getStr("label", "banned", "禁止"), 0);
    m_limit->addItem(ConfigManager::inst().getStr("label", "limited", "限制"), 1);
    m_limit->addItem(ConfigManager::inst().getStr("label", "semi-limited", "准限制"), 2);
    m_limit->addItem(ConfigManager::inst().getStr("label", "nolimited", "无限制"), 3);
    hbox1->addWidget(m_limit);

    m_ot = new QComboBox;
    m_ot->addItem("N/A", 0);
    m_ot->addItem("OCG", 1);
    m_ot->addItem("TCG", 2);
    hbox1->addWidget(m_ot);

    arr.set('e', hbox1);

    arrange arrM("ab|cd|ef|gh|ij|kl|mn|op", gridM);

    arrM.set2('a', raceL, 'b', m_cardRace);
    arrM.set2('c', attrL, 'd', m_cardAttr);
    arrM.set2('e', atkL, 'f', m_atkEdit);
    arrM.set2('g', defL, 'h', m_defEdit);
    arrM.set2('i', levelL, 'j', m_levelEdit);
    arrM.set2('k', rankL, 'l', m_rankEdit);
    arrM.set2('m', scaleL, 'n', m_scaleEdit);
    arrM.set2('o', linkL, 'p', m_linkEdit);

    auto wM = new QWidget;
    wM->setLayout(gridM);
    tab->addTab(wM, ConfigManager::inst().getStr("label", "attribute", "属性"));

    auto gridE = new QGridLayout;
    for(int i : range(32))
    {
        m_effects[i] = new QCheckBox;
        m_effects[i]->setText(ConfigManager::inst().getStr("effect", QString::number(i), ""));
        gridE->addWidget(m_effects[i], i / 2, i % 2);
    }

    auto wE = new QScrollArea;
    wE->setLayout(gridE);
    tab->addTab(wE, ConfigManager::inst().getStr("label", "effect", "效果"));

    arr.set('f', tab);
    arr.set2('g', setL, 'h', m_setEdit);
    arr.set2('i', nameL, 'j', m_nameEdit);

    auto searchButton = new IconButton(":/icons/searchall.png", ConfigManager::inst().getStr("action", "searchall", "搜索"));
    auto searchThisButton = new IconButton(":/icons/search.png", ConfigManager::inst().getStr("action", "searchthis", "在结果中搜索"));
    auto searchDeckButton = new IconButton(":/icons/search.png", ConfigManager::inst().getStr("action", "searchdeck", "在卡组中搜索"));
    auto revertButton = new IconButton(":/icons/revert.png", ConfigManager::inst().getStr("action", "revert", "复位"));
    arr.set('k', searchButton);

    auto hbox = new QHBoxLayout;
    hbox->addWidget(revertButton);
    hbox->addWidget(searchDeckButton);
    hbox->addWidget(searchThisButton);

    arr.set('l', hbox);

    m_inverseMode = new QCheckBox;
    m_inverseMode->setText(ConfigManager::inst().getStr("label", "inverse", "反选模式"));

    m_noSortMode = new QCheckBox;
    m_noSortMode->setText(ConfigManager::inst().getStr("label", "nosort", "不排序"));

    arr.set2('m', m_inverseMode, 'n', m_noSortMode);
    setLayout(grid);

    connect(m_cardType, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &CardFilter::setCardTypeSub);
    connect(searchButton, &QPushButton::clicked, this, &CardFilter::searchAll);
    connect(searchDeckButton, &QPushButton::clicked, this, &CardFilter::searchDeck);
    connect(searchThisButton, &QPushButton::clicked, this, &CardFilter::searchThis);
    connect(revertButton, &QPushButton::clicked, this, &CardFilter::revert);
}

void CardFilter::setCardTypeSub(int index)
{
    m_cardTypeSub->clear();
    m_cardTypeSub->addItem("N/A", ~0U);
    m_cardAttr->clear();
    m_cardAttr->addItem("N/A", ~0U);
    m_cardRace->clear();
    m_cardRace->addItem("N/A", ~0U);

    switch(index)
    {
    case 0:
        break;
    case 1:
        for(auto t : monsterTypes)
        {
            m_cardTypeSub->addItem(CardManager::inst().getType(t), QVariant(t));
        }
        for(auto t : monsterAttrs)
        {
            m_cardAttr->addItem(CardManager::inst().getAttr(t), QVariant(t));
        }
        for(auto t : monsterRaces)
        {
            m_cardRace->addItem(CardManager::inst().getRace(t), QVariant(t));
        }
        break;
    case 2:
        m_cardTypeSub->addItem(ConfigManager::inst().getStr("string", "TYPE_NORMAL", "通常"), 0);
        for(auto t : spellTypes)
        {
            m_cardTypeSub->addItem(CardManager::inst().getType(t), QVariant(t));
        }
        break;
    case 3:
        m_cardTypeSub->addItem(ConfigManager::inst().getStr("string", "TYPE_NORMAL", "通常"), 0);
        for(auto t : trapTypes)
        {
            m_cardTypeSub->addItem(CardManager::inst().getType(t), QVariant(t));
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
        for(; Spairp(res); res = Scdr(res))
        {
            list->append(Sunsigned_value(Scar(res)));
        }
    });

    if(!m_noSortMode->isChecked())
    {
        qSort(list->begin(), list->end(), idCompare);
    }

    emit result(list);
}

void CardFilter::searchAll()
{
    auto list = Type::DeckP::create();
    with_scheme([&]()
    {
        ptr pred = make_pred();
        ptr res = engine->call("search-all", pred, Sboolean(m_inverseMode->isChecked()));

        for(; res != Snil; res = Scdr(res))
        {
            list->append(Sunsigned_value(Scar(res)));
        }
    });

    if(!m_noSortMode->isChecked())
    {
        qSort(list->begin(), list->end(), idCompare);
    }

    emit result(list);

    return;
}

void CardFilter::searchThis()
{
    auto &t = m_getCurrent();
    search(t.begin(), t.end());
}

void CardFilter::searchDeck()
{
    auto deck = m_getDeck();
    search(deck->begin(), deck->end());
}

ptr CardFilter::make_pred()
{

    int type = m_cardType->currentData().toInt();
    int subtype = m_cardTypeSub->currentData().toInt();
    int race = m_cardRace->currentData().toInt();
    int attr = m_cardAttr->currentData().toInt();

    QString pass = m_passEdit->text();
    QString atk = m_atkEdit->text();
    QString def = m_defEdit->text();
    QString level = m_levelEdit->text();
    QString rank = m_rankEdit->text();
    QString scale = m_scaleEdit->text();
    QString link = m_linkEdit->text();

    quint32 category = 0;

    quint32 unit = 1;
    for(int i : range(32))
    {
        if(m_effects[i]->isChecked())
        {
            category |= unit;
        }
        unit = unit << 1;
    }

    int limitC = m_limit->currentData().toInt();
    int otC = m_ot->currentData().toInt();

    if(type == Const::TYPE_SPELL || type == Const::TYPE_TRAP)
    {
        subtype |= type;
    }

    QString setcode = m_setEdit->currentText();
    QString name = m_nameEdit->text();

    ptr param = Smake_vector(16, Sfalse);
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
    Svector_set(param, 15, engine->fromQString(link));

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
        ptr res = engine->call("search", ls, pred, Sboolean(m_inverseMode->isChecked()));

        Sunlock_object(pred);
        for(; res != Snil; res = Scdr(res))
        {
            list->append(Sunsigned_value(Scar(res)));
        }
    });

    if(!m_noSortMode->isChecked())
    {
        qSort(list->begin(), list->end(), idCompare);
    }

    emit result(list);

    return;
}

void CardFilter::revert()
{
    m_cardType->setCurrentIndex(0);
    m_cardTypeSub->setCurrentIndex(0);
    m_cardRace->setCurrentIndex(0);
    m_cardAttr->setCurrentIndex(0);
    m_setEdit->setCurrentIndex(0);


    m_passEdit->clear();
    m_atkEdit->clear();
    m_defEdit->clear();
    m_levelEdit->clear();
    m_rankEdit->clear();
    m_scaleEdit->clear();
    m_nameEdit->clear();

    for(auto effect : m_effects)
    {
        effect->setChecked(false);
    }
    m_inverseMode->setChecked(false);
}

CardFilter::~CardFilter()
{

}

