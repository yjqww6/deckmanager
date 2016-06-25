#include "expansions.h"
#include <QDebug>
#include <QPixmap>
#include <QMutex>
#include <QMutexLocker>

static QMutex mutex;

void Expansions::loadZips(QStringList zipFiles)
{
    foreach(QString fileName, zipFiles)
    {
        auto zipFile = QSharedPointer<QuaZip>::create(fileName);
        if(zipFile->open(QuaZip::mdUnzip))
        {
            m_zips.append(zipFile);
        }
    }
}

QByteArray Expansions::open(QString path)
{
    QByteArray arr;
    QMutexLocker locker(&mutex);
    foreach(auto zipFile, m_zips)
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
