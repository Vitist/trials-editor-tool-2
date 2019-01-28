#include "config.h"
#include <QFile>

Config::Config(): userId(""), platform("")
{
}

void Config::setConfig(QString userId, QString platform)
{
    this->userId = userId;
    this->platform = platform;
}

QMap<QString, QString> Config::load()
{
    QMap<QString, QString> config;
    config.insert("status", "success");

    // Check if config file has already been read
    if(userId.isEmpty() || platform.isEmpty()) {
        QFile file("config.txt");
        if(!file.open(QIODevice::ReadOnly)) {
            // Couldn't open file
            config.insert("status", "config_not_found");
        }

        QStringList validPlatforms = {"steam", "uplay"};
        while(!file.atEnd()) {
            QString line = file.readLine();
            QStringList lineKeyAndValue = line.split("=");
            if(lineKeyAndValue.size() == 2) {
                if(lineKeyAndValue.at(0) == "userId" && lineKeyAndValue.at(1).size() == 32) {
                    userId = lineKeyAndValue.at(1);
                }
                else if(lineKeyAndValue.at(0) == "platform" && validPlatforms.contains(lineKeyAndValue.at(1))) {
                    platform = lineKeyAndValue.at(1);
                }
            }
            else {
                // Something wrong with config file
                config.insert("status", "config_corrupted");
            }
        }
        file.close();
    }

    config.insert("userId", userId);
    config.insert("platform", platform);

    return config;
}

void Config::save()
{
    QFile file("config.txt");
    if(file.open(QIODevice::WriteOnly)) {
        file.write("userId=" + userId.toUtf8() + "\n");
        file.write("platform=" + platform.toUtf8());
    }
    file.close();
}
