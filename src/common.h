#ifndef COMMON_H
#define COMMON_H

template <typename T>
class enable_singleton
{
public:
    static T& inst()
    {
        static T in;
        return in;
    }
};

#endif // COMMON_H
