#include "funscripthandler.h"
#include <QIODevice>
#include <QFile>
#include <QIODevice>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>


FunscriptHandler::FunscriptHandler()
{

}


bool FunscriptHandler::load(QString funscript)
{
    QFile loadFile(funscript);

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open funscript file.");
        return false;
    }

    QByteArray funData = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(funData));

    FunscriptHandler::funscript = FunscriptHandler::JSonToFunscript(loadDoc.object());

    return true;
}

Funscript* FunscriptHandler::JSonToFunscript(QJsonObject json)
{
    Funscript* funscript = new Funscript();
    if (json.contains("range"))
        funscript->range = json["range"].toInt();
    if (json.contains("version") && json["version"].isString())
        funscript->version = json["version"].toString();
    if (json.contains("inverted") && json["inverted"].isBool())
        funscript->inverted = json["inverted"].toBool();
    if (json.contains("actions") && json["actions"].isArray())
    {
        QJsonArray actionArray = json["actions"].toArray();
        foreach(QJsonValue value, actionArray)
        {
            QJsonObject obj = value.toObject();
            if (json.contains("at") && json.contains("pos"))
            {
                funscript->actions.push_back({json["at"].toInt(), json["pos"].toInt()});
            }

        }
    }

    return funscript;
}

bool FunscriptHandler::exists(QString path)
{
    QFile fpath(path);
    return fpath.exists();
}
