#ifndef WRAPPER_H
#define WRAPPER_H
#include <QSharedPointer>

template<typename T>
class Wrapper
{
private:
    QSharedPointer<T> ptr;
public:
    Wrapper(QSharedPointer<T> p) : ptr(p) {}
    Wrapper() : ptr(QSharedPointer<T>(nullptr)) {}

    bool isNull() const
    {
        return ptr.isNull();
    }

    T& ref()
    {
        return *ptr.data();
    }
};


template<typename Func, typename T>
static inline void call_with_ref(Func f, Wrapper<T> w)
{
    if(!w.isNull())
    {
        auto t = std::forward<T&>(w.ref());
        f(t);
    }
    return;
}

template<typename Func, typename T, typename Ret>
static inline Ret call_with_def(Func f, Ret ret, Wrapper<T> w)
{
    if(!w.isNull())
    {
        auto t = std::forward<T&>(w.ref());
        return f(t);
    }
    else
    {
        return ret;
    }
}

#endif // WRAPPER_H

