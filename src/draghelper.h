#ifndef DRAGHELPER_H
#define DRAGHELPER_H
#include "common.h"

struct DragHelper : public enable_singleton<DragHelper>
{
    explicit DragHelper() : moved(false), atomic(false) {}
    bool moved;
    bool atomic;
};

#endif // DRAGHELPER_H
