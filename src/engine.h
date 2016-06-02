#ifndef ENGINE_H
#define ENGINE_H

#include "scheme.h"
#include <string>
#include <QString>
#include <QDebug>

class Engine
{
private:

    void registerSymbol_void(const char* sym, void* addr);
    void registerProc_void(const char* sym, void* addr, const char* spec);
    ptr except, myeval, except2str;

public:

    Engine(const char* argv0, const char* petite = nullptr, const char* scheme = nullptr);
    ~Engine();

    template <typename T>
    void registerSymbol(const char* sym, T* addr)
    {
        registerSymbol_void(sym, reinterpret_cast<void*>(addr));
    }

    template <typename T>
    void registerProc(const char* sym, T* addr, const char* spec)
    {
        registerProc_void(sym, reinterpret_cast<void*>(addr), spec);
    }

    bool isException(ptr val);
    ptr eval(const char* sexp);

    static std::string getString(ptr str);
    static std::string getBytes(ptr str);

    ptr getExcept() const;
    std::string getExceptStr(ptr e);
    std::string getExceptStr();

    ptr bytevector(const char* data, size_t len);
    ptr bytevector(const QByteArray& ba)
    {
        return bytevector(ba.data(), ba.length());
    }

    ptr fromQString(const QString& str);

    void putarg(size_t)
    {
    }

    template <typename First, typename... Rest>
    void putarg(size_t pos, First first, Rest... rest)
    {
        Sput_arg(pos, first);
        putarg(pos + 1, rest...);
    }

    template <typename... Args>
    ptr call(const char *sym, Args... args)
    {
        ptr f = Stop_level_value(Sstring_to_symbol(sym));

        if(!Sprocedurep(f))
        {
            Sset_top_level_value(except, Sstring("not a procedure"));
            qDebug() << sym << "not a procedure";
            return except;
        }

        Sinitframe(sizeof...(args));
        putarg(1, args...);
        ptr ret = Scall(f, sizeof...(args));

        if(isException(ret))
        {
            qDebug() << getExceptStr().c_str();
        }
        return ret;
    }

    void init();
};

class SchemeThreadActivator
{
public:
    SchemeThreadActivator()
    {
        Sactivate_thread();
    }

    ~SchemeThreadActivator()
    {
        Sdeactivate_thread();
    }
};

template <typename Proc>
auto with_scheme(Proc&& proc) -> decltype(std::forward<Proc>(proc)())
{
    SchemeThreadActivator act;
    return std::forward<Proc>(proc)();
}

extern Engine *engine;
#endif // ENGINE_H
