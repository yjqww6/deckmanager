#ifndef CONFIG_H
#define CONFIG_H

#include <QObject>
#include <QSettings>
#include "common.h"

class ConfigManager : public QObject, public enable_singleton<ConfigManager>
{
    Q_OBJECT
public:
    explicit ConfigManager(QObject *parent = 0);
    ~ConfigManager();
    QString getStr(QString group, QString key, QString defaultStr);
    QString getHelpStr();
    void loadConfig();
    void saveConfig();

signals:

public:

    bool m_waitForPass;
    bool m_convertPass;
    bool m_autoSwitch;
    bool m_bg;
    bool m_newTab;
    bool m_usesetname;
    int m_limit;

    QHash<QString, QVariant> m_configs;
    QList<QPair<int, QString> > m_deckTypes;
    QList<QPair<int, QString> > m_Flts;
    int m_deckType;
    int m_Flt;
    QString m_tempConfig;
};
#endif // CONFIG_H
