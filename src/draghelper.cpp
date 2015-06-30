#include "draghelper.h"

DragHelper dragHelper;

DragHelper::DragHelper(QObject *parent)
    : QObject(parent), moved(false), atomic(false)
{

}
