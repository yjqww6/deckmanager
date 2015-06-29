#include "mainwindow.h"
#include <QApplication>
#include <QLabel>
#include <QDebug>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QScrollBar>
#include <QFileInfo>
#include <QFont>
#include "yrp.h"
#include "expansions.h"
#include "limitcards.h"


int main(int argc, char *argv[])
{
    int ret = 0;
    QApplication a(argc, argv);
    try
    {
        Config configR;
        config = &configR;

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

        Expansions expansionsR(zipPaths);
        expansions = &expansionsR;

        CardPool::Load(paths);

        LimitCards::load();

        MainWindow w;
        w.show();
        CardPool::LoadNames();
        ret =  a.exec();
    }
    catch(...)
    {

    }
    return ret;
}
