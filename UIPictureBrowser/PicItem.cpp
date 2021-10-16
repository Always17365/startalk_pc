//
// Created by cc on 2018/11/17.
//

#include "PicItem.h"
#include <QPainter>
#include <cmath>
#include <iostream>
#include <QMovie>
#include <QFileInfo>
#include <QGraphicsSceneMouseEvent>

PicItem::PicItem(int &scaleVal)
    : _scaleVal(scaleVal)
{
}

PicItem::~PicItem()
{
    _pixmap = QPixmap();
    _pMovie->deleteLater();
}

QRectF PicItem::boundingRect() const
{
    if (!_pixmap.isNull())
    {
        qreal width = _pixmap.width() * _proportion;
        qreal height = _pixmap.height() * _proportion;
        return {-width / 2, -height / 2, width, height};
    }

    return {};
}

void PicItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (!_pixmap.isNull() && nullptr != _pMovie)
    {
        QMutexLocker locker(&_mutex);
        painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
        qreal width = _pixmap.width() * _proportion;
        qreal height = _pixmap.height() * _proportion;

        if(_pMovie->isValid())
        {
            painter->drawPixmap(static_cast<int>(-width / 2), static_cast<int>(-height / 2), static_cast<int>(width),
                                static_cast<int>(height), _pMovie->currentPixmap());
        }
        else
        {
            painter->drawPixmap(static_cast<int>(-width / 2), static_cast<int>(-height / 2), static_cast<int>(width),
                                static_cast<int>(height), _pixmap
                               );
        }
    }
}

// 设置新图片
void PicItem::setPixmap(QPixmap &pixmap, const QString &path, qreal proportion)
{
    QMutexLocker locker(&_mutex);

    if (nullptr != _pMovie)
    {
        _pMovie->stop();
        delete _pMovie;
    }

    _pMovie = new QMovie(path);
    _pMovie->start();
    QObject::connect(_pMovie, &QMovie::frameChanged, [this](int)
    {
        this->update();
    });
    _pixmap = pixmap;
    _proportion = proportion;
}

// 图片缩放
void PicItem::onScaleChange(int scaleVal, const QPoint &p)
{
    if (scaleVal > maxScale)
    {
        _scaleVal = maxScale;
        return;
    }

    if (scaleVal < -maxScale)
    {
        _scaleVal = -maxScale;
        return;
    }

    qreal multiple;
#ifdef Q_OS_MAC
    static const qreal sca = 1.05;
#else
    static const qreal sca = 1.1;
#endif

    if (scaleVal > 0)
        multiple = pow(sca, scaleVal);
    else
        multiple = pow(1 / sca, -scaleVal);

    setTransformOriginPoint(p);
    setScale(multiple);
}
