#include "xsubtitlesource.h"

#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

// Parse an SRT/VTT timestamp (HH:MM:SS,mmm or HH:MM:SS.mmm) into milliseconds.
static qint64 parseSrtTimestamp(const QString &ts)
{
    QString cleaned = ts.trimmed().replace(',', '.');
    QStringList parts = cleaned.split(':');
    if (parts.size() < 3)
        return -1;
    int hours = parts[0].toInt();
    int minutes = parts[1].toInt();
    QStringList secParts = parts[2].split('.');
    int seconds = secParts[0].toInt();
    int ms = 0;
    if (secParts.size() > 1)
        ms = secParts[1].left(3).toInt();
    return ((qint64)hours * 3600 + minutes * 60 + seconds) * 1000 + ms;
}

// Parse an ASS/SSA timestamp (H:MM:SS.cc, centiseconds) into milliseconds.
static qint64 parseAssTimestamp(const QString &ts)
{
    QString cleaned = ts.trimmed();
    QStringList parts = cleaned.split(':');
    if (parts.size() < 3)
        return -1;
    int hours = parts[0].toInt();
    int minutes = parts[1].toInt();
    QStringList secParts = parts[2].split('.');
    int seconds = secParts[0].toInt();
    int cs = 0;
    if (secParts.size() > 1)
        cs = secParts[1].left(2).toInt();
    return ((qint64)hours * 3600 + minutes * 60 + seconds) * 1000 + cs * 10;
}

// Strip inline markup (HTML tags, ASS override blocks) from a cue line.
static QString cleanCueText(QString text)
{
    text.remove(QRegularExpression("<[^>]*>"));          // HTML tags
    text.remove(QRegularExpression("\\{[^}]*\\}"));      // ASS override blocks
    text.replace(QRegularExpression("\\r?\\n"), "\n");   // normalize newlines
    return text.trimmed();
}

XSubtitleSource::XSubtitleSource(QObject *parent)
    : QObject(parent)
{
}

QStringList XSubtitleSource::findSidecars(const QString &mediaFile) const
{
    QStringList result;
    QFileInfo fi(mediaFile);
    if (!fi.exists())
        return result;
    QString base = fi.absolutePath() + "/" + fi.completeBaseName();
    const QStringList extensions = { "srt", "vtt", "ass", "ssa" };
    for (const QString &ext : extensions) {
        QString path = base + "." + ext;
        if (QFile::exists(path))
            result << fi.completeBaseName() + "." + ext;
    }
    return result;
}

QString XSubtitleSource::sidecarPath(const QString &mediaFile, const QString &baseName) const
{
    QFileInfo fi(mediaFile);
    QString path = fi.absolutePath() + "/" + baseName;
    if (QFile::exists(path))
        return path;
    return QString();
}

bool XSubtitleSource::load(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;
    QString content = QString::fromUtf8(file.readAll());
    file.close();

    clear();
    m_currentFile = filePath;

    const QString ext = QFileInfo(filePath).suffix().toLower();
    if (ext == "srt")
        parseSrt(content);
    else if (ext == "vtt")
        parseVtt(content);
    else if (ext == "ass" || ext == "ssa")
        parseAss(content);

    return !m_cues.isEmpty();
}

QString XSubtitleSource::cueAt(qint64 positionMs) const
{
    for (const SubtitleCue &cue : m_cues) {
        if (positionMs >= cue.startMs && positionMs < cue.endMs)
            return cue.text;
    }
    return QString();
}

void XSubtitleSource::clear()
{
    m_cues.clear();
    m_currentFile.clear();
}

void XSubtitleSource::parseSrt(const QString &content)
{
    // SRT blocks are separated by blank lines:
    //   1
    //   00:00:01,000 --> 00:00:04,000
    //   Text line 1
    //   Text line 2
    QRegularExpression arrowRe("(\\d{1,2}:\\d{2}:\\d{2}[,.]\\d{1,3})\\s*-->\\s*(\\d{1,2}:\\d{2}:\\d{2}[,.]\\d{1,3})");
    const QStringList blocks = content.split(QRegularExpression("[\\r\\n]+\\s*[\\r\\n]+"), Qt::SkipEmptyParts);
    for (const QString &block : blocks) {
        QRegularExpressionMatch match = arrowRe.match(block);
        if (!match.hasMatch())
            continue;
        qint64 start = parseSrtTimestamp(match.captured(1));
        qint64 end = parseSrtTimestamp(match.captured(2));
        if (start < 0 || end <= start)
            continue;
        int textStart = block.indexOf(match.captured(2)) + match.captured(2).length();
        QString text = cleanCueText(block.mid(textStart));
        if (!text.isEmpty())
            m_cues.append({ start, end, text });
    }
}

void XSubtitleSource::parseVtt(const QString &content)
{
    // VTT is like SRT but starts with a "WEBVTT" header and uses '.' decimals.
    QString cleaned = content;
    int header = cleaned.indexOf("WEBVTT");
    if (header >= 0) {
        int newline = cleaned.indexOf('\n', header);
        if (newline >= 0)
            cleaned = cleaned.mid(newline + 1);
    }
    parseSrt(cleaned);
}

void XSubtitleSource::parseAss(const QString &content)
{
    // ASS/SSA [Events] section:
    //   Format: Layer, Start, End, Style, Name, MarginL, MarginR, MarginV, Effect, Text
    //   Dialogue: 0,0:00:01.00,0:00:04.00,Default,,0,0,0,,Text
    bool inEvents = false;
    int textIndex = -1;
    for (const QString &rawLine : content.split('\n')) {
        const QString line = rawLine.trimmed();
        if (line.startsWith("[Events]")) {
            inEvents = true;
            continue;
        }
        if (!inEvents)
            continue;
        if (line.startsWith("Format:")) {
            const QStringList fields = line.mid(7).split(',');
            for (int i = 0; i < fields.size(); ++i) {
                if (fields[i].trimmed() == "Text") {
                    textIndex = i;
                    break;
                }
            }
            continue;
        }
        if (line.startsWith("Dialogue:")) {
            if (textIndex < 0)
                textIndex = 9; // default ASS layout
            // Split keeping the text field intact (it may contain commas).
            const QString data = line.mid(9);
            QStringList fields = data.split(QChar(','), Qt::KeepEmptyParts);
            // The text field (last) may contain commas; rejoin any overflow.
            if(fields.size() > textIndex + 1) {
                QString text = fields.mid(textIndex + 1).join(QChar(','));
                QStringList head = fields.mid(0, textIndex + 1);
                head.append(text);
                fields = head;
            }
            if (fields.size() <= textIndex)
                continue;
            qint64 start = parseAssTimestamp(fields[1]);
            qint64 end = parseAssTimestamp(fields[2]);
            if (start < 0 || end <= start)
                continue;
            QString text = cleanCueText(fields[textIndex]);
            text.replace("\\N", "\n").replace("\\n", "\n");
            if (!text.isEmpty())
                m_cues.append({ start, end, text });
        }
    }
}