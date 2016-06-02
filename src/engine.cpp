#include "engine.h"
#include <QRegExp>
#include <QMutex>
#include <QStringList>
#include <QDebug>
#include <QTextCodec>
#include <vector>
#include "card.h"
#include "expansions.h"
#include "curl/curl.h"
#include "networking.h"
#include "limitcards.h"

static void custom_init() {}

static size_t write_data(void * ptr, size_t size, size_t nmemb, void * stream)
{
    std::vector<char> &data = *reinterpret_cast<std::vector<char>*>(stream);
    size_t block = size * nmemb;
    data.reserve(data.size() + block);
    for(size_t i = 0; i < block; ++i)
    {
        data.push_back(reinterpret_cast<char*>(ptr)[i]);
    }
    return block;
}

static ptr download(ptr url, size_t timeout)
{
    if(!Sstringp(url))
    {
        return Sfalse;
    }
    std::string surl = engine->getString(url);

    Sdeactivate_thread();

    CURL *curl = curl_easy_init();

    std::vector<char> data;

    curl_easy_setopt(curl, CURLOPT_URL, surl.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

    curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    Sactivate_thread();
    return engine->bytevector(data.data(), data.size());
}

static void qdebug(ptr str)
{
    qDebug() << engine->getString(str).c_str();
}

static ptr expansion_open(const char* path)
{
    return engine->bytevector(expansions->open(path));
}

static ptr readFile(const char* path)
{
    QFile file(path);
    if(file.open(QFile::ReadOnly))
    {
        return engine->bytevector(file.readAll());
    }
    else
    {
        return Sfalse;
    }
}

static ptr getAllCards()
{
    ptr vec = Smake_vector(cardPool->pool.size(), Sfalse);

    int i = 0;
    for(auto it = cardPool->pool.begin(); it != cardPool->pool.end(); ++it, ++i)
    {
        Svector_set(vec, i, Sunsigned(it->first));
    }
    return vec;
}

static int cardLimit(size_t id)
{
    return limitCards->getLimit(id);
}

static ptr bytesConvert(ptr bytes, const char* from, const char* to)
{
    QTextCodec* codecFrom = QTextCodec::codecForName(from);
    QTextCodec* codecTo = QTextCodec::codecForName(to);

    if(!codecFrom || !codecTo)
    {
        return Sfalse;
    }

    std::string b = engine->getBytes(bytes);
    return engine->bytevector(codecTo->fromUnicode(codecFrom->toUnicode(QByteArray(b.data(), b.length()))));
}

static ptr regex(ptr pat, ptr str, size_t offset, int minimal)
{
    char *data = reinterpret_cast<char*>(Sbytevector_data(pat));
    QRegExp reg(QString::fromUtf8(data, Sbytevector_length(pat)));
    reg.setMinimal(minimal != 0);

    data = reinterpret_cast<char*>(Sbytevector_data(str));

    QString qstr = QString::fromUtf8(data + offset, Sbytevector_length(str) - offset);

    int pos = reg.indexIn(qstr);

    const QStringList& ls = reg.capturedTexts();

    //qDebug() << pat << str << pos << len << ls;

    if(pos == -1)
    {
        return Sfalse;
    }

    int actual_pos = qstr.left(pos).toUtf8().length();

    ptr vec = Smake_vector(ls.length(), Sfalse);

    for(int i = 0; i < ls.length(); ++i)
    {
        const auto& ba = ls[i].toUtf8();
        Svector_set(vec, i, engine->bytevector(ba.data(), ba.length()));
    }

    return Scons(Sfixnum(actual_pos), vec);
}

static ptr getCard(int id)
{
    auto card = cardPool->getCard(id);
    if(card.isNull())
    {
        return Sfalse;
    }
    else
    {
        Card& c = card;
        std::string exp = "(make-ftype-pointer card " + std::to_string(reinterpret_cast<size_t>(&c)) + ")";
        return engine->eval(exp.c_str());
    }
}

static ptr getCardName(Card *card)
{
    if(!card)
    {
        return Sstring("");
    }

    ptr b = engine->bytevector(card->name.toUtf8());
    return engine->call("utf8->string", b);
}

static ptr getCardEffect(Card *card)
{
    if(!card)
    {
        return Sstring("");
    }

    ptr b = engine->bytevector(card->effect.toUtf8());
    return engine->call("utf8->string", b);
}

Engine::Engine(const char* argv0, const char* petite, const char* scheme)
{
    Sscheme_init(nullptr);
    if(petite)
    {
        Sregister_boot_file(petite);
    }
    else
    {
        Sregister_boot_file("./petite.boot");
    }

    if(scheme)
    {
        Sregister_boot_file(scheme);
    }
    else
    {
        Sregister_boot_file("./scheme.boot");
    }

    Sbuild_heap(argv0, custom_init);

    ptr gensym = Stop_level_value(Sstring_to_symbol("gensym"));
    except = Scall0(gensym);
    Slock_object(except);

    const char* sexp =
            "(lambda (s e)"
            "  (call/1cc"
            "    (lambda (k)"
            "      (with-exception-handler "
            "        (lambda (x) (set-top-level-value! e x) (k e))"
            "        (lambda () (eval (read (open-input-string s)))) ) ) ) )";

    ptr ois = Stop_level_value(Sstring_to_symbol("open-input-string"));
    ptr sip = Scall1(ois, Sstring(sexp));
    ptr read = Stop_level_value(Sstring_to_symbol("read"));
    ptr expr = Scall1(read, sip);
    ptr _eval = Stop_level_value(Sstring_to_symbol("eval"));
    myeval = Scall1(_eval, expr);
    Slock_object(myeval);


    except2str = eval("(lambda (e)"
                      "  (call-with-string-output-port"
                      "    (lambda (p)"
                      "      (if (condition? e)"
                      "          (display-condition e p)"
                      "          (display e p)))))");
    Slock_object(except2str);
}

Engine::~Engine()
{
    Sscheme_deinit();
}

bool Engine::isException(ptr val)
{
    return val == except;
}

ptr Engine::eval(const char *sexp)
{
    ptr p = Scall2(myeval, Sstring(sexp), except);
    if(isException(p))
    {
        qDebug() << getExceptStr().c_str();
    }
    return p;
}

std::string Engine::getString(ptr str)
{
    if(!Sstringp(str))
    {
        return "";
    }

    ptr bytes = Scall1(Stop_level_value(Sstring_to_symbol("string->utf8")), str);

    int len = Sbytevector_length(bytes);
    std::string buf((char*)Sbytevector_data(bytes), len);
    return std::move(buf);
}

std::string Engine::getBytes(ptr str)
{
    if(!Sbytevectorp(str))
    {
        return "";
    }

    int len = Sbytevector_length(str);
    std::string buf((char*)Sbytevector_data(str), len);
    return std::move(buf);
}

void Engine::registerSymbol_void(const char *sym, void *addr)
{
    Sregister_symbol(sym, addr);
}

void Engine::registerProc_void(const char *sym, void *addr, const char *spec)
{
    Sregister_symbol(sym, addr);
    std::string exp("(define ");
    exp.append(sym);
    exp.append("(foreign-procedure \"");
    exp.append(sym);
    exp.append("\" ");
    exp.append(spec);
    exp.append("))");
    eval(exp.c_str());
}

ptr Engine::getExcept() const
{
    return except;
}

std::string Engine::getExceptStr(ptr e)
{
    return getString(Scall1(except2str, e));
}

std::string Engine::getExceptStr()
{
    return getString(Scall1(except2str, Stop_level_value(except)));
}

ptr Engine::bytevector(const char *data, size_t len)
{
    ptr b = Smake_bytevector(len, 0);
    for(size_t i = 0; i < len; ++i)
    {
        Sbytevector_u8_set(b, i, data[i]);
    }
    return b;
}

ptr Engine::fromQString(const QString &str)
{
    return engine->call("utf8->string", bytevector(str.toUtf8()));
}

void Engine::init()
{
    registerProc("regex", regex, "(ptr ptr size_t boolean) ptr");
    registerProc("bytes-convert", bytesConvert, "(ptr string string) ptr");
    registerProc("get-card", getCard, "(int) ptr");
    registerProc("expansion-open", expansion_open, "(string) ptr");
    registerProc("get-all-cards", getAllCards, "() ptr");
    registerProc("qdebug", qdebug, "(ptr) void");
    registerProc("read-file", readFile, "(string) ptr");

    eval("(load \"./pack/init.ss\")");

    registerProc("card-name", getCardName, "((* card)) ptr");
    registerProc("card-effect", getCardEffect, "((* card)) ptr");
    registerProc("card-limit", cardLimit, "(size_t) int");
    registerProc("download", download, "(ptr size_t) ptr");
    registerProc("notify", notify, "(size_t int int ptr) void");
}
