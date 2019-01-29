#ifndef CONFIG_H
#define CONFIG_H

#include <QMap>
#include <QString>
#include <QDir>

class Config
{
public:
    Config();

    bool initialize(QDir saveDir);
    QMap<QString, QString> getConfig();
    bool load();
    void save();

private:
    QString userId;
};

#endif // CONFIG_H
