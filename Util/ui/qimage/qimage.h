//
// Created by cc on 19-1-16.
//

#ifndef STALK_V2_QIMAGE_H
#define STALK_V2_QIMAGE_H

#include <QObject>
#include <QMap>
#include <QPixmap>
#include <QMutexLocker>
#include "Util/util_global.h"

namespace st {

class STALK_UTIL_EXPORT qimage
{

public:
    static QPixmap loadImage(const QString &srcPath, bool save, bool scaled = false,
                             int width = 0, int height = 0);
    static QPixmap loadCirclePixmap(const QString &srcPath, const int &radius,
                                    bool isGrey = false);
    static QPixmap generateGreyPixmap(const QPixmap &src);
    static QString getRealImageSuffix(const QString &filePath);
    static QString getRealImagePath(const QString &filePath);
    static QPixmap scaledPixmap(const QPixmap &src, int width, int height = 0,
                                bool model = false);
    static QPixmap generatePixmap(const QPixmap &src, const int &radius);
    static int dpi();

    static QString getGifImagePath(const QString &srcPath, qreal &width,
                                   qreal &height);
    static QString getGifImagePathNoMark(const QString &srcPath);
};
}


#endif //STALK_V2_QIMAGE_H
