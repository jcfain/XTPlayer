#ifndef XSUBTITLESOURCE_H
#define XSUBTITLESOURCE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QList>

// A single subtitle cue with a start/end time (ms) and display text.
struct SubtitleCue {
    qint64 startMs = 0;
    qint64 endMs = 0;
    QString text;
};

// Parses sidecar subtitle files (.srt / .vtt / .ass / .ssa) that live next to
// a media file. Qt 6 removed QMediaSubtitle, so embedded-track text can no
// longer be read from QMediaPlayer; this class covers the common case of
// distributed sidecar subtitles and drives the on-screen subtitle label.
class XSubtitleSource : public QObject
{
    Q_OBJECT
public:
    explicit XSubtitleSource(QObject *parent = nullptr);

    // Return the base names of sidecar subtitle files found next to mediaFile
    // (e.g. "movie.srt"). Order: srt, vtt, ass, ssa.
    QStringList findSidecars(const QString &mediaFile) const;

    // Absolute path of the sidecar file for a given base name, or empty.
    QString sidecarPath(const QString &mediaFile, const QString &baseName) const;

    // Load and parse a subtitle file. Returns true if at least one cue parsed.
    bool load(const QString &filePath);

    // The cue text active at positionMs, or an empty string if none.
    QString cueAt(qint64 positionMs) const;

    int cueCount() const { return m_cues.size(); }
    void clear();

private:
    void parseSrt(const QString &content);
    void parseVtt(const QString &content);
    void parseAss(const QString &content);

    QList<SubtitleCue> m_cues;
    QString m_currentFile;
};

#endif // XSUBTITLESOURCE_H