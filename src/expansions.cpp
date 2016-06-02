#include "expansions.h"
#include <QDebug>
#include <QPixmap>
#include <QMutex>
#include <QMutexLocker>

Expansions *expansions = nullptr;

static QMutex mutex;

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
    QByteArray arr;
    //QMutexLocker locker(&mutex);
    foreach(auto zipFile, zips)
    {
        if(zipFile->setCurrentFile(path))
        {
            QuaZipFile file(zipFile.data());

            if(file.open(QIODevice::ReadOnly))
            {
                arr = file.readAll();
                break;
            }
        }
    }
    return std::move(arr);
}
