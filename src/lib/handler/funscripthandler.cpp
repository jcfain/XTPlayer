#include "funscripthandler.h"


Funscript* funscript = new Funscript();
int lastAction;

FunscriptHandler::FunscriptHandler()
{

}

FunscriptHandler::~FunscriptHandler()
{
}

bool FunscriptHandler::load(QString funscriptString)
{
    QFile loadFile(funscriptString);

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open funscript file.");
        return false;
    }

    QByteArray funData = loadFile.readAll();
    QJsonParseError* error = new QJsonParseError();
    QJsonDocument doc = QJsonDocument::fromJson(funData, error);

    if(doc.isNull()) {
        emit errorOccurred("loading funscript failed: " + error->errorString());
        return false;
    }
    delete error;
    JSonToFunscript(doc.object());

    return true;
}

void FunscriptHandler::JSonToFunscript(QJsonObject json)
{
    if (json.contains("range"))
        funscript->range = json["range"].toInt();
    if (json.contains("version") && json["version"].isString())
        funscript->version = json["version"].toString();
    if (json.contains("inverted") && json["inverted"].isBool())
        funscript->inverted = json["inverted"].toBool();
    if (json.contains("actions") && json["actions"].isArray())
    {
        funscript->actions.clear();
        QJsonArray actionArray = json["actions"].toArray();
        foreach(QJsonValue value, actionArray)
        {
            QJsonObject obj = value.toObject();
            if (obj.contains("at") && obj.contains("pos"))
            {
                funscript->actions[(qint64)obj["at"].toDouble()] = obj["pos"].toInt();
            }

        }
    }
}

bool FunscriptHandler::exists(QString path)
{
    QFile fpath(path);
    return fpath.exists();
}

int FunscriptHandler::getPosition(qint64 millis)
{
    if (funscript->actions.contains(millis))
    {
        lastAction = funscript->actions.value(millis);
        return lastAction;
    }
    return lastAction;
}
