#include "funscripthandler.h"

FunscriptHandler::FunscriptHandler(QString channel)
{
    _channel = channel;
}

FunscriptHandler::~FunscriptHandler()
{
    delete(funscript);
    _loaded = false;
    SettingsHandler::setFunscriptLoaded(_channel, _loaded);
}

QString FunscriptHandler::channel()
{
    return _channel;
}

bool FunscriptHandler::load(QString funscriptString)
{
    QMutexLocker locker(&mutex);
    if (funscriptString == nullptr)
    {
        _loaded = false;
        LogHandler::Error("Loading funscript failed: Empty string" );
        SettingsHandler::setFunscriptLoaded(_channel, _loaded);
        return false;
    }
    LogHandler::Debug("Funscript load: "+funscriptString);
    QFile loadFile(funscriptString);
    lastActionIndex = -1;
    nextActionIndex = 1;
    if (!loadFile.open(QIODevice::ReadOnly)) {
        LogHandler::Warn("Loading funscript failed: Couldn't open funscript file.");
        _loaded = false;
        SettingsHandler::setFunscriptLoaded(_channel, _loaded);
        return false;
    }

    //LogHandler::Debug("funscriptHandler->load "+QString::number((round(timer.nsecsElapsed()) / 1000000)));
    QByteArray funData = loadFile.readAll();
    locker.unlock();
    return load(funData);
}

bool FunscriptHandler::load(QByteArray byteArray)
{
    QMutexLocker locker(&mutex);
    _firstActionExecuted = false;
    _funscriptMax = -1;
    QJsonParseError* error = new QJsonParseError();
    QJsonDocument doc = QJsonDocument::fromJson(byteArray, error);

    if(doc.isNull()) {
        LogHandler::Error("Loading funscript JSON failed: " + error->errorString());
        delete error;
        _loaded = false;
        SettingsHandler::setFunscriptLoaded(_channel, _loaded);
        return false;
    }
    delete error;
    JSonToFunscript(doc.object());
    _funscriptMax = posList.length() > 0 ? posList.last() : -1;
    _funscriptMin =  posList.length() > 0 ? posList.first() > 0 || posList.length() == 1 ? posList.first() : posList.value(1) : 0;
    _funscriptMin = 0;
    foreach(qint64 value, posList)
    {
        if(value > 0)
        {
            _funscriptMin = value;
            break;
        }
    }

    _loaded = true;
    SettingsHandler::setFunscriptLoaded(_channel, _loaded);
    return true;
}

Funscript* FunscriptHandler::currentFunscript()
{
    QMutexLocker locker(&mutex);
    return funscript;
}

bool FunscriptHandler::isLoaded()
{
    QMutexLocker locker(&mutex);
    return _loaded;
}

void FunscriptHandler::setLoaded(bool value)
{
    QMutexLocker locker(&mutex);
    _loaded = value;
    SettingsHandler::setFunscriptLoaded(_channel, _loaded);
}

qint64 FunscriptHandler::getMin()
{
    return _funscriptMin;
}

qint64 FunscriptHandler::getMax()
{
    return _funscriptMax;
}

void FunscriptHandler::JSonToFunscript(QJsonObject json)
{
    if (json.contains("range"))
        funscript->range = json["range"].toInt();
    if (json.contains("version") && json["version"].isString())
        funscript->version = json["version"].toString();
    if (json.contains("inverted") && json["inverted"].isBool())
    {
        _inverted = json["inverted"].toBool();
        funscript->inverted = json["inverted"].toBool();
    }
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
    if (json.contains("creator") && json["creator"].isString())
        funscript->metadata.creator = json["creator"].toString();
    if (json.contains("original_name") && json["original_name"].isString())
        funscript->metadata.original_name = json["original_name"].toString();
    if (json.contains("url") && json["url"].isString())
        funscript->metadata.url = json["url"].toString();
    if (json.contains("url_video") && json["url_video"].isString())
        funscript->metadata.url_video = json["url_video"].toString();
    if (json.contains("tags") && json["tags"].isArray())
    {
        QJsonArray tags = json["tags"].toArray();;
        foreach(QJsonValue tag, tags)
            funscript->metadata.tags.append(tag.toString());
    }
    if (json.contains("performers") && json["performers"].isArray())
    {
        QJsonArray performers = json["performers"].toArray();;
        foreach(QJsonValue performer, performers)
            funscript->metadata.performers.append(performer.toString());
    }
    if (json.contains("paid") && json["paid"].isBool())
        funscript->metadata.paid = json["paid"].toBool();
    if (json.contains("comment") && json["comment"].isString())
        funscript->metadata.comment = json["comment"].toString();
    if (json.contains("original_total_duration_ms"))
        funscript->metadata.original_total_duration_ms = json["original_total_duration_ms"].toString().toLongLong();
}

bool FunscriptHandler::exists(QString path)
{
    QFile fpath(path);
    return fpath.exists();
}

std::shared_ptr<FunscriptAction> FunscriptHandler::getPosition(qint64 millis)
{
    QMutexLocker locker(&mutex);
    millis += SettingsHandler::getLiveOffSet() == 0 ? SettingsHandler::getoffSet() : SettingsHandler::getLiveOffSet();
    qint64 closestMillis = findClosest(millis, posList);
    if(closestMillis == -1)
        return nullptr;
    nextActionIndex = posList.indexOf(closestMillis) + 1;
    if(nextActionIndex >= posList.length())
        return nullptr;
    qint64 nextMillis = posList[nextActionIndex];
    //LogHandler::Debug("millis: "+ QString::number(millis));
    //LogHandler::Debug("closestMillis: "+ QString::number(closestMillis));
    //LogHandler::Debug("lastActionIndex: "+ QString::number(lastActionIndex));
    //LogHandler::Debug("nextActionIndex: "+ QString::number(nextActionIndex));
//    LogHandler::Debug("nextMillis: "+ QString::number(nextMillis));
    if ((lastActionIndex != nextActionIndex && millis >= closestMillis) || lastActionIndex == -1)
    {
        int interval = lastActionIndex == -1 ? closestMillis : nextMillis - closestMillis;
        if(!_firstActionExecuted)
        {
            _firstActionExecuted = true;
            if(interval < 500)
                interval = 500;
        }
        //LogHandler::Debug("offSet: "+ QString::number(SettingsHandler::getoffSet()));
        //LogHandler::Debug("speed: "+ QString::number(speed));
        //LogHandler::Debug("millis: "+ QString::number(millis));
//        LogHandler::Debug("closestMillis: "+ QString::number(closestMillis));
//        LogHandler::Debug("nextMillis: "+ QString::number(nextMillis));
//        LogHandler::Debug("lastActionIndex: "+ QString::number(lastActionIndex));
//        LogHandler::Debug("nextActionIndex: "+ QString::number(nextActionIndex));
        //LogHandler::Debug("nextActionPos: "+ QString::number(funscript->actions.value(nextMillis)));
        qint64 executionMillis = lastActionIndex == -1 ? closestMillis : nextMillis;
        std::shared_ptr<FunscriptAction> nextAction(new FunscriptAction { _channel, executionMillis, funscript->actions.value(executionMillis), interval, lastActionPos, lastActionSpeed });
        //LogHandler::Debug("nextAction.speed: "+ QString::number(nextAction->speed));
        lastActionIndex = nextActionIndex;
        lastActionPos = funscript->actions.value(executionMillis);
        lastActionSpeed = interval;
        return nextAction;
    }
    return nullptr;
}

qint64 FunscriptHandler::findClosest(qint64 value, QList<qint64> a) {

    if(a.length() == 0)
        return -1;
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

//static
bool FunscriptHandler::getInverted()
{
    QMutexLocker locker(&mutexStat);
    return _inverted;
}
void FunscriptHandler::setInverted(bool value)
{
    QMutexLocker locker(&mutexStat);
    _inverted = value;
}

bool FunscriptHandler::_inverted = false;
QMutex FunscriptHandler::mutexStat;
