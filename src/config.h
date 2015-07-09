#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QSettings>


typedef QHash<QString, QVariant> Map;

class RemoteConfig
{
public:
    RemoteConfig(Map&, QString, RemoteConfig *);
    RemoteConfig(QString);
    QString id;
    QString str;
    QString codec;
    QString getlist;
    QString getlistparam;
    QString finishlist;
    QString deckname;
    QString deckid;
    QString decktype;
    QString decktooltip;
    QString getdeck;
    QString finishdeck;
    QString deck;
    QString getname;
    QString finishname;
    QString name;
    QString openurl;
    bool operator ==(const RemoteConfig &other)
    {
        return id == other.id;
    }
    RemoteConfig& operator =(const RemoteConfig &other);
};

class Config : public QObject
{
    Q_OBJECT
public:
    explicit Config(QObject *parent = 0);
    ~Config();
    bool waitForPass;
    bool convertPass;
    bool autoSwitch;
    bool bg;
    int limit;
    QString getStr(QString group, QString key, QString defaultStr);
    QString getHelpStr();
    RemoteConfig& getCurrentRemote();
    QList<RemoteConfig> remoteConfigs;
    int remote;
    Map mappings;
signals:

public slots:
    void setWaitForPass(bool);
    void setConvertPass(bool);
    void setLimit(int);
    void setAutoSwitch(bool);
    void setRemote(int);
};

extern Config *config;
extern RemoteConfig tempRemoteConfig;
#endif // CONFIG_H
