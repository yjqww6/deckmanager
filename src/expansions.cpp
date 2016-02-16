#include "expansions.h"
#include <QDebug>
#include <QPixmap>

Expansions *expansions = nullptr;
Expansions::Expansions(QStringList zipFiles)
{
    foreach(QString fileName, zipFiles)
    {
        auto zipFile = QSharedPointer<QuaZip>::create(fileName);
        if(zipFile->open(QuaZip::mdUnzip))
        {
            zips.append(zipFile);
        }
    }
}

QByteArray Expansions::open(QString path)
{
    bool found = false;
    QByteArray arr;
    foreach(auto zipFile, zips)
    {
        if(found)
        {
            break;
        }
        if(zipFile->setCurrentFile(path))
        {
            QuaZipFile file(zipFile.data());

            if(file.open(QIODevice::ReadOnly))
            {
                found = true;
                arr = file.readAll();

            }
            file.close();
        }
    }
    return arr;
}
