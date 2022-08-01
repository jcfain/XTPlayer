#include "xlibrarylistwidget.h"

XLibraryListWidget::XLibraryListWidget(QWidget* parent) : QListWidget(parent)
{

}
void XLibraryListWidget::keyPressEvent(QKeyEvent *event) {
    emit keyPressed(event);
}
