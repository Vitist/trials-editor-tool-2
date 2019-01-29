#include "config.h"
#include <QFile>
#include <QDebug>
#include <QRegularExpression>

Config::Config(): userId(""), platform("")
{
}

void Config::setConfig(QString userId, QString platform)
{
    this->userId = userId;
    this->platform = platform;
}

QMap<QString, QString> Config::getConfig()
{
    QMap<QString, QString> config;
    config.insert("userId", userId);
    config.insert("platform", platform);
    return config;
}

bool Config::load()
{
    // Check if config file has already been read
    if(userId.isEmpty() || platform.isEmpty()) {
        QFile file("config.txt");
        if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            // Couldn't open file
            return false;
        }

        // Steam and uplay hex strings
        QStringList validPlatforms = {"cafeb00b", "deadbabe"};
        // User id hex string matcher
        QRegularExpression hexMatcher("^[0-9A-F]{32}$", QRegularExpression::CaseInsensitiveOption);
        while(!file.atEnd()) {
            QString line = file.readLine().trimmed();
            QStringList lineKeyAndValue = line.split("=");
            // Make sure the config file lines are valid
            if(lineKeyAndValue.size() == 2) {
                QRegularExpressionMatch match = hexMatcher.match(lineKeyAndValue.at(1));
                if(lineKeyAndValue.at(0) == "userId" && match.hasMatch()) {
                    userId = lineKeyAndValue.at(1);
                    qDebug() << "userId: " << userId;
                }
                else if(lineKeyAndValue.at(0) == "platform" && validPlatforms.contains(lineKeyAndValue.at(1))) {
                    platform = lineKeyAndValue.at(1);
                    qDebug() << "platform: " << platform;
                }
                else {
                    // Something wrong with config file
                    return false;
                }
            }
            else {
                // Something wrong with config file
                return false;
            }
        }
        file.close();
    }

    return true;
}

void Config::save()
{
    QFile file("config.txt");
    if(file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write("userId=" + userId.toUtf8() + "\n");
        file.write("platform=" + platform.toUtf8());
    }
    file.close();
}
