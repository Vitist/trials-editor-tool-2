#ifndef CONFIG_H
#define CONFIG_H

#include <QMap>
#include <QString>

class Config
{
public:
    Config();

    void setConfig(QString userId, QString platform);
    QMap<QString, QString> load();
    void save();

private:
    QString userId;
    QString platform;
};

#endif // CONFIG_H
