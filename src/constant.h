#ifndef CONSTANT_H
#define CONSTANT_H
#include <QtCore>

namespace Const
{
static const quint32 TYPE_MONSTER = 0x1;
static const quint32 TYPE_SPELL = 0x2;
static const quint32 TYPE_TRAP = 0x4;
static const quint32 TYPE_NORMAL = 0x10;
static const quint32 TYPE_EFFECT = 0x20;
static const quint32 TYPE_FUSION = 0x40;
static const quint32 TYPE_RITUAL = 0x80;
static const quint32 TYPE_TRAPMONSTER = 0x100;
static const quint32 TYPE_SPIRIT = 0x200;
static const quint32 TYPE_UNION = 0x400;
static const quint32 TYPE_DUAL = 0x800;
static const quint32 TYPE_TUNER = 0x1000;
static const quint32 TYPE_SYNCHRO = 0x2000;
static const quint32 TYPE_TOKEN = 0x4000;
static const quint32 TYPE_QUICKPLAY = 0x10000;
static const quint32 TYPE_CONTINUOUS = 0x20000;
static const quint32 TYPE_EQUIP = 0x40000;
static const quint32 TYPE_FIELD = 0x80000;
static const quint32 TYPE_COUNTER = 0x100000;
static const quint32 TYPE_FLIP = 0x200000;
static const quint32 TYPE_TOON = 0x400000;
static const quint32 TYPE_XYZ = 0x800000;
static const quint32 TYPE_PENDULUM =0x1000000;
static const quint32 TYPE_SPSUMMON =0x2000000;
static const quint32 TYPE_LINK =0x4000000;

static const quint32 RACE_WARRIOR = 0x1;
static const quint32 RACE_SPELLCASTER = 0x2;
static const quint32 RACE_FAIRY = 0x4;
static const quint32 RACE_FIEND = 0x8;
static const quint32 RACE_ZOMBIE = 0x10;
static const quint32 RACE_MACHINE = 0x20;
static const quint32 RACE_AQUA = 0x40;
static const quint32 RACE_PYRO = 0x80;
static const quint32 RACE_ROCK = 0x100;
static const quint32 RACE_WINDBEAST = 0x200;
static const quint32 RACE_PLANT = 0x400;
static const quint32 RACE_INSECT = 0x800;
static const quint32 RACE_THUNDER = 0x1000;
static const quint32 RACE_DRAGON = 0x2000;
static const quint32 RACE_BEAST = 0x4000;
static const quint32 RACE_BEASTWARRIOR = 0x8000;
static const quint32 RACE_DINOSAUR = 0x10000;
static const quint32 RACE_FISH = 0x20000;
static const quint32 RACE_SEASERPENT = 0x40000;
static const quint32 RACE_REPTILE = 0x80000;
static const quint32 RACE_PSYCHO = 0x100000;
static const quint32 RACE_DEVINE = 0x200000;
static const quint32 RACE_CREATORGOD = 0x400000;
static const quint32 RACE_WYRM = 0x800000;
static const quint32 RACE_CYBERS = 0x1000000;

static const quint32 ATTRIBUTE_EARTH = 0x01;
static const quint32 ATTRIBUTE_WATER = 0x02;
static const quint32 ATTRIBUTE_FIRE = 0x04;
static const quint32 ATTRIBUTE_WIND = 0x08;
static const quint32 ATTRIBUTE_LIGHT = 0x10;
static const quint32 ATTRIBUTE_DARK = 0x20;
static const quint32 ATTRIBUTE_DEVINE = 0x40;
}
#endif // CONSTANT_H

