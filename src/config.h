#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QSettings>


typedef QHash<QString, QVariant> Map;

class RemoteConfig
{
public:
    RemoteConfig();
    void set(Map &mappings, QString group);
    QString codec;
    QString getlist;
    QString getlistparam;
    QString getlistparamwithdecktype;
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
    bool newTab;
    bool usesetname;
    int limit;
    QString getStr(QString group, QString key, QString defaultStr);
    QString getHelpStr();
    RemoteConfig& getCurrentRemote();
    RemoteConfig remoteConfig;
    Map mappings;
    QList<QPair<int, QString> > deckTypes;
    QList<QPair<int, QString> > Flts;
    int deckType;
    int Flt;
    QString tempConfig;
signals:

public slots:
    void setWaitForPass(bool);
    void setConvertPass(bool);
    void setLimit(int);
    void setAutoSwitch(bool);
    void setNewTab(bool value);
    void setUseSetName(bool);
};

extern Config *config;
#endif // CONFIG_H
