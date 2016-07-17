#include "mainwindow.h"
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <iostream>
#include "yrp.h"
#include "expansions.h"
#include "limitcards.h"
#include "configmanager.h"
#include "card.h"
#include "carditem.h"
#include "signaltower.h"
#include "engine.h"
#include "curl/curl.h"
#include <thread>

Engine *engine = nullptr;

int main(int argc, char *argv[])
{
    int ret = 0;
    QApplication a(argc, argv);
    try
    {
        Engine eng(argv[0], "./pack/petite.boot", "./pack/scheme.boot");
        engine = &eng;
        eng.init();
        Sdeactivate_thread();

        ConfigManager::inst().loadConfig();

        QDir base("."), ext("expansions");
        QStringList paths, filters;
        filters << "*.cdb";
        foreach(auto info, ext.entryInfoList(filters))
        {
            paths << info.filePath();
        }
        foreach(auto info, base.entryInfoList(filters))
        {
            paths << info.filePath();
        }

        QStringList zipFilters, zipPaths;
        zipFilters << "*.zip";
        foreach(auto info, ext.entryInfoList(zipFilters))
        {
            zipPaths << info.filePath();
        }

        Expansions::inst().loadZips(zipPaths);

        CardManager::inst().loadCardData(paths);

        LimitCards::inst().load();

        curl_global_init(CURL_GLOBAL_ALL);

        with_scheme([&]()
        {
            eng.call("load-setnames");
        });

        MainWindow w;

        SignalTower::inst().m_mainLoaded = true;
        w.show();

        if(ConfigManager::inst().getStr("pref", "scanscript", "1") == "1")
        {
            with_scheme([&]()
            {
                eng.call("load-setnames");
            });
        }

        ret =  a.exec();
        ConfigManager::inst().saveConfig();
    }
    catch(...)
    {

    }
    return ret;
}
