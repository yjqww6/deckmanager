#ifndef DRAGHELPER_H
#define DRAGHELPER_H

struct DragHelper
{
    explicit DragHelper() : moved(false), atomic(false) {}
    bool moved;
    bool atomic;
};

extern DragHelper dragHelper;

#endif // DRAGHELPER_H
