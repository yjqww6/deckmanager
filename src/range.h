#ifndef RANGE_H
#define RANGE_H
#include <type_traits>

template<typename T>
class Range
{
public:
    Range(T __begin, T __end)
        : _begin(__begin), _end(__end)
    {
    }

    typedef T value_type;

    class iterator
    {
    public:
        typedef T value_type;
        iterator(T i) : pos(i) {}
        const T& operator ++()
        {
            return ++pos;
        }

        bool operator !=(const iterator &other) const
        {
            return pos != other.pos;
        }

        T operator *() const
        {
            return pos;
        }

    private:
        value_type pos;
    };

    const iterator begin() const
    {
        return iterator(_begin);
    }

    const iterator end() const
    {
        return iterator(_end);
    }

private:
    T _begin, _end;
};

template<typename T>
static inline Range<T> range(T __begin, T __end)
{
    return std::move(Range<T>(__begin, __end));
}

template<typename T>
static inline Range<T> range(T __end)
{
    return std::move(Range<T>(static_cast<T>(0), __end));
}

#endif // RANGE_H

