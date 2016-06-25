#include "packlist.h"
#include "configmanager.h"
#include "engine.h"
#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include <quazip/quazipdir.h>
#include <QTextDocument>

PackList::PackList(QWidget *parent) : QTreeWidget(parent), m_myPack(nullptr)
{
    setColumnCount(2);
    setColumnHidden(1, true);
    setHeaderHidden(true);
    m_encoding = ConfigManager::inst().getStr("pack", "zipencoding", "gbk");
    connect(this, &PackList::itemClicked, this, &PackList::readPackClk);
}

void PackList::refresh()
{
    QuaZip zip("pack/pack.zip");
    zip.setFileNameCodec(m_encoding.toLatin1().data());
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
                auto ls = subDir.entryInfoList();
                std::sort(ls.begin(), ls.end(), [](const QuaZipFileInfo &a, const QuaZipFileInfo &b)
                {
                    return a.name < b.name;
                });
                foreach(QuaZipFileInfo itemInfo, ls)
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
    zip.close();

    qSort(items.begin(), items.end(), [&](QTreeWidgetItem *a, QTreeWidgetItem *b) {
        return a->text(0) < b->text(0);
    });

    QDir dir("pack/" + ConfigManager::inst().getStr("pack", "mypackpath", "__我的卡包") + "/");
    m_myPack = new QTreeWidgetItem;
    m_myPack->setText(0, ConfigManager::inst().getStr("pack", "mypack", "我的卡包"));
    foreach(QFileInfo info, dir.entryInfoList(filter))
    {
        auto item = new QTreeWidgetItem;
        item->setText(0, info.completeBaseName());
        item->setData(0, Qt::UserRole, info.filePath());
        item->setData(1, Qt::UserRole, false);
        m_myPack->insertChild(m_myPack->childCount(), item);
    }
    items.prepend(m_myPack);
    insertTopLevelItems(0, items);
}

void PackList::readPack(QTreeWidgetItem *item, int)
{
    if(!item || item->parent() == nullptr)
    {
        return;
    }

    QString data;
    QString name;

    name = item->text(0);

    if(item && item->parent() != m_myPack)
    {
        QuaZip zip("pack/pack.zip");
        zip.setFileNameCodec(m_encoding.toLatin1().data());
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
    auto vec = Type::DeckP::create();

    for(QString line = stream.readLine(); !line.isNull();
        line = stream.readLine())
    {
        if(line.length() <= 1)
        {
            continue;
        }
        int pos = line.indexOf('-');
        if(pos > 0)
        {
            quint32 id = line.left(pos).toUInt();

            if(auto ocard = CardManager::inst().getCard(id))
            {
                Card &card = **ocard;
                vec->append(card.id);
            }
            else
            {
                line = line.mid(pos + 1);
                line = line.trimmed();
                if(auto ocard = CardManager::inst().getNewCard(line, ConfigManager::inst().m_waitForPass))
                {
                    Card &card = **ocard;
                    vec->append(card.id);
                }
            }
        }
        else if(pos < 0)
        {
            quint32 id = line.toUInt();
            if(auto ocard = CardManager::inst().getCard(id))
            {
                Card &card = **ocard;
                vec->append(card.id);
            }
        }
        else
        {
            line = line.mid(pos + 1);
            line = line.trimmed();
            if(auto ocard = CardManager::inst().getNewCard(line, ConfigManager::inst().m_waitForPass))
            {
                Card &card = **ocard;
                vec->append(card.id);
            }
        }
    }

    if(m_edited)
    {
        emit editCards(vec);
        emit packName(name);
    }
    else
    {
        emit cards(vec);
    }
}


RemotePackList::RemotePackList(QWidget *parent) : PackList(parent), m_net(make_networking())
{
    connect(m_net.get(), &NetWorking::packList, this, &RemotePackList::packList, Qt::QueuedConnection);
    connect(m_net.get(), &NetWorking::pack, this, &RemotePackList::pack, Qt::QueuedConnection);
}

void RemotePackList::refresh()
{
    m_net->getPackList();
}

void RemotePackList::readPack(QTreeWidgetItem *item, int)
{

    if(!item || item->parent() == nullptr)
    {
        return;
    }

    int no = item->data(0, Qt::UserRole).toInt();
    m_net->getPack(no);

    if(m_edited)
    {
        emit packName(item->text(0));
    }
}

void RemotePackList::packList(ptr ls)
{

    QList<QTreeWidgetItem*> items;
    with_scheme([&]()
    {
        Sunlock_object(ls);
        for(ptr i = ls; Spairp(i); i = Scdr(i))
        {
            auto pack = new QTreeWidgetItem;
            pack->setText(0, engine->getString(Scar(Scar(i))));
            items.append(pack);

            for(ptr l = Scdr(Scar(i)); Spairp(l); l = Scdr(l))
            {
                auto item = new QTreeWidgetItem;
                item->setText(0, engine->getString(Scar(Scar(l))));
                item->setData(0, Qt::UserRole, Sfixnum_value(Scdr(Scar(l))));
                pack->insertChild(pack->childCount(), item);
                Sunlock_object(l);
            }
            Sunlock_object(i);
        }
    });
    clear();
    insertTopLevelItems(0, items);
}

void RemotePackList::pack(ptr ls)
{
    auto vec = Type::DeckP::create();
    with_scheme([&]()
    {
        Sunlock_object(ls);
        for(; Spairp(ls); ls = Scdr(ls))
        {
            vec->append(Sunsigned32_value(Scar(ls)));
        }
    });

    if(m_edited)
    {
        emit editCards(vec);
    }
    else
    {
        emit cards(vec);
    }
}
