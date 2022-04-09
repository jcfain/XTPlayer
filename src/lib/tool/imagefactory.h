#ifndef IMAGEFACTORY_H
#define IMAGEFACTORY_H

#include <QPixmap>

class ImageFactory
{
public:
    ImageFactory();
public:
    static QPixmap* resize(QString filepath, QSize thumbSize);
private:
    static QSize calculateMaxSize(QSize size);
};

#endif // IMAGEFACTORY_H
