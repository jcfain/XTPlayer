#ifndef XMEDIA_H
#define XMEDIA_H
enum XMediaStatus {
    LoadingMedia,
    LoadedMedia,
    NoMedia,
    BufferingMedia,
    BufferedMedia,
    UnknownMediaStatus,
    StalledMedia,
    InvalidMedia,
    EndOfMedia

};
enum XMediaState {
    Paused,
    Playing,
    Stopped,
};
#endif // XMEDIA_H
