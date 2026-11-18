#include "fullscreenmodaldialog.h"

#include <QApplication>

FullScreenModalDialog* FullScreenModalDialog::s_instance = nullptr;
int FullScreenModalDialog::s_count = 0;
QWidget* FullScreenModalDialog::s_fullScreenWindow = nullptr;
QWidget* FullScreenModalDialog::s_dialogParent = nullptr;

FullScreenModalDialog::FullScreenModalDialog() : QObject(nullptr) {}

FullScreenModalDialog* FullScreenModalDialog::instance()
{
    if (!s_instance)
        s_instance = new FullScreenModalDialog();
    return s_instance;
}

void FullScreenModalDialog::setFullScreenWindow(QWidget* window)
{
    s_fullScreenWindow = window;
}

void FullScreenModalDialog::setDialogParent(QWidget* widget)
{
    s_dialogParent = widget;
}

void FullScreenModalDialog::beginModal(QDialog* dialog)
{
    if (!dialog)
        return;

    if (s_fullScreenWindow) {
        // Keep the dialog above the full screen video window.
        dialog->setWindowFlags(dialog->windowFlags() | Qt::WindowStaysOnTopHint);
        // A window-modal dialog only blocks its parent's window, not the
        // separate full screen video window. Make it application-modal so the
        // video window is blocked and cannot be raised over the dialog.
        dialog->setWindowModality(Qt::ApplicationModal);
        // The full screen player hides the cursor; show it while a dialog is open.
        qApp->setOverrideCursor(Qt::ArrowCursor);
    }

    s_count++;
    emit instance()->modalStateChanged(s_count);
}

void FullScreenModalDialog::endModal(QDialog* dialog)
{
    Q_UNUSED(dialog);
    s_count = qMax(0, s_count - 1);
    emit instance()->modalStateChanged(s_count);
}