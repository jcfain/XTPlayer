#ifndef FULLSCREENMODALDIALOG_H
#define FULLSCREENMODALDIALOG_H

#include <QObject>
#include <QDialog>

// Tracks modal dialogs opened while the video is in full screen so that they
// stay on top of the full screen video window and the mouse cursor stays
// visible.
//
// The full screen video view is a separate top-level window (see
// VideoHandler::showFullscreen), so dialogs parented to the main window are
// neither blocked by (nor stacked above) it: clicking the video raises it over
// the dialog and the player's hidden-cursor override keeps the pointer blank.
// Marking the dialog stays-on-top + application-modal fixes both - it stacks
// above the video and its modality blocks the video window from being raised.
class FullScreenModalDialog : public QObject
{
    Q_OBJECT
public:
    static FullScreenModalDialog* instance();

    // Call before showing a modal dialog (exec). While the video is in full
    // screen this marks the dialog stays-on-top + application-modal and shows
    // the cursor.
    static void beginModal(QDialog* dialog);

    // Call after the modal dialog closes (exec returns). When the last modal
    // closes, lets the player hide the cursor and re-grab the keyboard again.
    static void endModal(QDialog* dialog);

    static bool isModalOpen() { return s_count > 0; }

    // Set by VideoHandler when entering/leaving full screen (nullptr to clear).
    static void setFullScreenWindow(QWidget* window);
    static QWidget* fullScreenWindow() { return s_fullScreenWindow; }

    // A widget that lives inside the full screen video window (e.g. the library
    // list). Dialogs parented to it become child windows of the video window and
    // therefore stack on top of it. Set/cleared by VideoHandler.
    static void setDialogParent(QWidget* widget);
    // Returns the full screen dialog parent if in full screen, else fallback.
    static QWidget* dialogParent(QWidget* fallback) {
        return s_dialogParent ? s_dialogParent : fallback;
    }

signals:
    void modalStateChanged(int count);

private:
    FullScreenModalDialog();

    static FullScreenModalDialog* s_instance;
    static int s_count;
    static QWidget* s_fullScreenWindow;
    static QWidget* s_dialogParent;
};

#endif // FULLSCREENMODALDIALOG_H