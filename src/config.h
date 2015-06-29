#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QSettings>

class Config : public QObject
{
    Q_OBJECT
public:
    explicit Config(QObject *parent = 0);
    ~Config();
    bool waitForPass;
    bool convertPass;
    bool autoSwitch;
    int limit;
    typedef QHash<QString, QVariant> Map;
    QString getStr(QString group, QString key, QString defaultStr);
    QString getHelpStr();

    Map mappings;


signals:

public slots:
    void setWaitForPass(bool);
    void setConvertPass(bool);
    void setLimit(int);
    void setAutoSwitch(bool);
};

extern Config *config;
#endif // CONFIG_H
