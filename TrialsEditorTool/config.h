#ifndef CONFIG_H
#define CONFIG_H

#include <QMap>
#include <QString>
#include <QDir>

class Config
{
public:
    enum Game {
        Rising,
        Fusion
    };

    Config();

    bool initialize(QDir saveDir, Game game);
    QMap<QString, QString> getConfig();
    bool load();
    void save();

private:
    QString userId;
};

#endif // CONFIG_H
