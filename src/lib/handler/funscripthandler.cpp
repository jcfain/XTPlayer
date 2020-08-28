#include "funscripthandler.h"


Funscript* funscript = new Funscript();
qint64 lastActionIndex;
qint64  nextActionIndex;

 QList<qint64> posList;
 int n;
FunscriptHandler::FunscriptHandler()
{
}
FunscriptHandler::~FunscriptHandler()
{
    delete(funscript);
}

bool FunscriptHandler::load(QString funscriptString)
{
    QFile loadFile(funscriptString);

    lastActionIndex = -1;
    nextActionIndex = 1;
    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open funscript file.");
        return false;
    }

    QByteArray funData = loadFile.readAll();
    QJsonParseError* error = new QJsonParseError();
    QJsonDocument doc = QJsonDocument::fromJson(funData, error);

    if(doc.isNull()) {
        emit errorOccurred("loading funscript failed: " + error->errorString());
        delete error;
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
        posList = funscript->actions.keys();
        std::sort(posList.begin(), posList.end());
        n = posList.length() / sizeof(posList.first());
    }
}

bool FunscriptHandler::exists(QString path)
{
    QFile fpath(path);
    return fpath.exists();
}

std::unique_ptr<FunscriptAction> FunscriptHandler::getPosition(qint64 millis)
{
    QMutexLocker locker(&mutex);
    millis += SettingsHandler::getoffSet();
    qint64 closestMillis = findClosest(millis, posList);
    nextActionIndex = posList.indexOf(closestMillis) + 1;
    qint64 nextMillis = posList[nextActionIndex];
    if ((lastActionIndex != nextActionIndex && millis >= closestMillis) || lastActionIndex == -1)
    {
        int speed = lastActionIndex == -1 ? closestMillis : nextMillis - closestMillis;
        //LogHandler::Debug("millis: "+ QString::number(millis));
        //LogHandler::Debug("closestMillis: "+ QString::number(closestMillis));
        //LogHandler::Debug("speed: "+ QString::number(speed));
        //LogHandler::Debug("nextMillis: "+ QString::number(nextMillis));
        //LogHandler::Debug("lastActionIndex: "+ QString::number(lastActionIndex));
        //LogHandler::Debug("nextActionIndex: "+ QString::number(nextActionIndex));
        //LogHandler::Debug("nextActionPos: "+ QString::number(funscript->actions.value(nextMillis)));
        qint64 executionMillis = lastActionIndex == -1 ? closestMillis : nextMillis;
        std::unique_ptr<FunscriptAction> nextAction(new FunscriptAction { executionMillis, funscript->actions.value(executionMillis), speed } );
        lastActionIndex++;
        return nextAction;
    }
    return nullptr;
}

qint64 FunscriptHandler::findClosest(qint64 value, QList<qint64> a) {

      if(value < a[0]) {
          return a[0];
      }
      if(value > a[a.length()-1]) {
          return a[a.length()-1];
      }

      int lo = 0;
      int hi = a.length() - 1;

      while (lo <= hi) {
          int mid = (hi + lo) / 2;

          if (value < a[mid]) {
              hi = mid - 1;
          } else if (value > a[mid]) {
              lo = mid + 1;
          } else {
              return a[mid];
          }
      }
      // lo == hi + 1
      return (a[lo] - value) < (value - a[hi]) ? a[lo] : a[hi];
  }

QMutex FunscriptHandler::mutex;
