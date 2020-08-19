#include "funscripthandler.h"


Funscript* funscript = new Funscript();
FunscriptAction* lastAction = new FunscriptAction { 0, 0, 0 };
 QList<qint64> posList;
 int n;
FunscriptHandler::FunscriptHandler()
{
}
FunscriptHandler::~FunscriptHandler()
{
    delete(funscript);
    delete(lastAction);
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

FunscriptAction* FunscriptHandler::getPosition(qint64 millis)
{
    qint64 nearestActionMillis = findClosest(posList, n, millis);
    int pos = funscript->actions.value(nearestActionMillis);
    if (lastAction->pos != pos)
    {
        LogHandler::Debug("millis: "+ QString::number(millis));
        LogHandler::Debug("nearestAction: "+ QString::number(nearestActionMillis));
        LogHandler::Debug("lastAction->at: "+ QString::number(lastAction->at));
        int speed = -(nearestActionMillis - lastAction->at);
        LogHandler::Debug("speed: "+ QString::number(-speed));
        delete(lastAction);
        lastAction = new FunscriptAction { nearestActionMillis, pos, -speed };
        return lastAction;
    }
    return nullptr;
}

// Returns element closest to target in arr[]
qint64 FunscriptHandler::findClosest(QList<qint64> arr, qint64 n, qint64 target)
{
    // Corner cases
    if (target <= arr[0])
        return arr[0];
    if (target >= arr[n - 1])
        return arr[n - 1];

    // Doing binary search
    int i = 0, j = n, mid = 0;
    while (i < j) {
        mid = (i + j) / 2;

        if (arr[mid] == target)
            return arr[mid];

        /* If target is less than array element,
            then search in left */
        if (target < arr[mid]) {

            // If target is greater than previous
            // to mid, return closest of two
            if (mid > 0 && target > arr[mid - 1])
                return getClosest(arr[mid - 1],
                                  arr[mid], target);

            /* Repeat for left half */
            j = mid;
        }

        // If target is greater than mid
        else {
            if (mid < n - 1 && target < arr[mid + 1])
                return getClosest(arr[mid],
                                  arr[mid + 1], target);
            // update i
            i = mid + 1;
        }
    }

    // Only single element left after search
    return arr[mid];
}

// Method to compare which one is the more close.
// We find the closest by taking the difference
// between the target and both values. It assumes
// that val2 is greater than val1 and target lies
// between these two.
qint64 FunscriptHandler::getClosest(qint64 val1, qint64 val2, qint64 target)
{
    if (target - val1 >= val2 - target)
        return val2;
    else
        return val1;
}
