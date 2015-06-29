#include "packlist.h"
#include "config.h"
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include <quazip/quazipdir.h>
#include <QTextDocument>

PackList::PackList(QWidget *parent) : QTreeWidget(parent), myPack(nullptr)
{
    setColumnCount(2);
    setColumnHidden(1, true);
    setHeaderHidden(true);
    encoding = config->getStr("pack", "zipencoding", "gbk");
    connect(this, SIGNAL(itemClicked(QTreeWidgetItem*,int)),
            this, SLOT(readPackClk(QTreeWidgetItem*,int)));
}

void PackList::refresh()
{
    QuaZip zip("pack/pack.zip");
    zip.setFileNameCodec(encoding.toLatin1().data());
    QStringList filter;
    filter << "*.ypk";

    clear();

    QList<QTreeWidgetItem*> items;

    if(zip.open(QuaZip::mdUnzip))
    {
        QuaZipDir dir(&zip, "");
        foreach(QuaZipFileInfo info, dir.entryInfoList())
        {
            QuaZipDir subDir(&zip, info.name);
            if(info.name.endsWith("/") && subDir.exists())
            {

                auto pack = new QTreeWidgetItem;
                pack->setText(0, subDir.dirName());
                items.append(pack);
                foreach(QuaZipFileInfo itemInfo, subDir.entryInfoList())
                {
                    auto item = new QTreeWidgetItem;
                    QFileInfo temp(itemInfo.name);
                    QString name = temp.completeBaseName();
                    int pos = name.indexOf('-');
                    if(pos > 0)
                    {
                        name = name.mid(pos + 1);
                    }
                    QTextDocument doc;
                    doc.setHtml(name);
                    name = doc.toPlainText();
                    QString path = subDir.filePath(itemInfo.name);
                    item->setText(0, name);
                    item->setData(0, Qt::UserRole, path);
                    item->setData(1, Qt::UserRole, false);
                    pack->insertChild(pack->childCount(), item);
                }
            }
        }
    }
    qSort(items.begin(), items.end(), [&](QTreeWidgetItem *a, QTreeWidgetItem *b) {
        return a->text(0) < b->text(0);
    });
    zip.close();

    QDir dir("pack/" + config->getStr("pack", "mypackpath", "__我的卡包") + "/");
    myPack = new QTreeWidgetItem;
    myPack->setText(0, config->getStr("pack", "mypack", "我的卡包"));
    foreach(QFileInfo info, dir.entryInfoList(filter))
    {
        auto item = new QTreeWidgetItem;
        item->setText(0, info.completeBaseName());
        item->setData(0, Qt::UserRole, info.filePath());
        item->setData(1, Qt::UserRole, false);
        myPack->insertChild(myPack->childCount(), item);
    }
    items.prepend(myPack);
    insertTopLevelItems(0, items);
}

void PackList::readPack(QTreeWidgetItem *item, int)
{
    if(item->parent() == nullptr)
    {
        return;
    }

    QString data;
    QString name;

    name = item->text(0);

    if(item && item->parent() != myPack)
    {
        QuaZip zip("pack/pack.zip");
        zip.setFileNameCodec(encoding.toLatin1().data());
        if(zip.open(QuaZip::mdUnzip))
        {
            QString path = item->data(0, Qt::UserRole).toString();
            zip.setCurrentFile(path);
            QuaZipFile file(&zip);
            if(file.open(QIODevice::ReadOnly | QFile::Text))
            {
                data = file.readAll();
                file.close();
            }
        }
    }
    else
    {
        QString path = item->data(0, Qt::UserRole).toString();
        QFile file(path);
        if(file.open(QIODevice::ReadOnly))
        {
            data = file.readAll();
            file.close();
        }
    }

    QTextStream stream(&data);
    auto vec = QSharedPointer<QVector<int> >::create();

    for(QString line = stream.readLine(); !line.isNull() && line.length() > 1;
        line = stream.readLine())
    {
        int pos = line.indexOf('-');
        if(pos > 0)
        {
            int id = line.left(pos).toInt();
            auto card = CardPool::getCard(id);
            if(card)
            {
                vec->append(card->id);
            }
            else
            {
                line = line.mid(pos + 1);
                line = line.trimmed();
                auto card = CardPool::getNewCard(line, config->waitForPass);
                if(card)
                {
                    vec->append(card->id);
                }
            }
        }
        else if(pos < 0)
        {
            int id = line.toInt();
            auto card = CardPool::getCard(id);
            if(card)
            {
                vec->append(card->id);
            }
        }
        else
        {
            line = line.mid(pos + 1);
            line = line.trimmed();
            auto card = CardPool::getNewCard(line, config->waitForPass);
            if(card)
            {
                vec->append(card->id);
            }
        }
    }

    if(edited)
    {
        emit editCards(vec);
    }
    else
    {
        emit cards(vec);
    }
    emit packName(name);
}

