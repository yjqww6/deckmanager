#include "expansions.h"
#include <QDebug>
#include <QPixmap>

Expansions *expansions = nullptr;
Expansions::Expansions(QStringList zipFiles)
    : zips(zipFiles)
{
}

QByteArray Expansions::open(QString path)
{
    bool found = false;
    QByteArray arr;
    foreach(auto &zipFile, zips)
    {
        if(found)
        {
            break;
        }
        QuaZip zip(zipFile);
        if(zip.open(QuaZip::mdUnzip))
        {
            if(zip.setCurrentFile(path))
            {
                QuaZipFile file(&zip);

                if(file.open(QIODevice::ReadOnly))
                {
                    found = true;
                    arr = file.readAll();

                }
                file.close();
            }
            zip.close();
        }
    }
    return arr;
}
