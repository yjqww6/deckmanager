#include "engine.h"
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
#include "signaltower.h"

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
    std::string surl = engine->getString(url).toStdString();

    Sdeactivate_thread();


    std::vector<char> data;

    CURL *curl = curl_easy_init();
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
    QString qstr = engine->getString(str);
    qDebug() << qstr;
    if(SignalTower::inst().m_mainLoaded)
    {
        SignalTower::inst().schemeDebug(qstr);
    }
    else
    {
        SignalTower::inst().m_accumulated.append(qstr);
    }
}

static ptr expansion_open(const char* path)
{
    return engine->bytevector(Expansions::inst().open(path));
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
    ptr vec = Smake_vector(CardManager::inst().m_cards.size(), Sfalse);

    int i = 0;
    for(auto it = CardManager::inst().m_cards.begin(); it != CardManager::inst().m_cards.end(); ++it, ++i)
    {
        Svector_set(vec, i, Sunsigned(it->first));
    }
    return vec;
}

static int cardLimit(size_t id)
{
    return LimitCards::inst().getLimit(id);
}

static ptr bytesConvert(ptr bytes, const char* from, const char* to)
{
    QTextCodec* codecFrom = QTextCodec::codecForName(from);
    QTextCodec* codecTo = QTextCodec::codecForName(to);

    if(!codecFrom || !codecTo)
    {
        return Sfalse;
    }

    QByteArray ba = engine->getBytes(bytes);
    return engine->bytevector(codecTo->fromUnicode(codecFrom->toUnicode(ba)));
}

static ptr getCard(int id)
{
    auto card = CardManager::inst().getCard(id);
    if(!card)
    {
        return Sfalse;
    }
    else
    {
        Card& c = **card;
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
    return engine->fromQString(card->name);
}

static ptr getCardEffect(Card *card)
{
    if(!card)
    {
        return Sstring("");
    }

    return engine->fromQString(card->effect);
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
    ptr cmd = Scall1(Stop_level_value(Sstring_to_symbol("utf8->string")), bytevector(sexp));
    ptr p = Scall2(myeval, cmd, except);
    if(isException(p))
    {
        QString str = getExceptStr();
        qDebug() << str;
        if(SignalTower::inst().m_mainLoaded)
        {
            SignalTower::inst().schemeDebug(str);
        }
        else
        {
            SignalTower::inst().m_accumulated.append(str);
        }
    }
    return p;
}

QString Engine::getString(ptr str)
{
    if(!Sstringp(str))
    {
        return "";
    }

    int len = Sstring_length(str);
    QString buf(len, QChar(0));

    for(int i = 0; i < len; ++i)
    {
        buf[i] = Sstring_ref(str, i);
    }
    return std::move(buf);
}

QByteArray Engine::getBytes(ptr str)
{
    if(!Sbytevectorp(str))
    {
        return "";
    }

    int len = Sbytevector_length(str);
    QByteArray buf((char*)Sbytevector_data(str), len);
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

QString Engine::getExceptStr(ptr e)
{
    return getString(Scall1(except2str, e));
}

QString Engine::getExceptStr()
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
    ptr buf = Smake_string(str.length(), 0);
    for(int i = 0; i < str.length(); ++i)
    {
        Sstring_set(buf, i, (uint32_t)str[i].unicode());
    }
    return buf;
}

void Engine::init()
{
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
