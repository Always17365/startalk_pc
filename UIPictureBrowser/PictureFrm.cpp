//
// Created by cc on 2018/11/16.
//

#include "PictureFrm.h"
#include <QGridLayout>
#include <QDesktopWidget>
#include <QApplication>
#include <iostream>
#include <QDateTime>
#include <cmath>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QDebug>
#include "PictureBrowser.h"
#include "../Platform/Platform.h"
#include "../UICom/uicom.h"
#include "../QtUtil/Utils/Log.h"
#include "../UICom/qimage/qimage.h"
#include "../CustomUi/QtMessageBox.h"


PictureFrm::PictureFrm(PictureBrowser *pBrowser)
    : QFrame(pBrowser), _pPicBrowser(pBrowser), _pPicItem(nullptr), _scaleVal(0), _angle(0),
      _pressed(false)
{
    _pGraphicsScene = new QGraphicsScene(this);
    _pGraphicsView = new QGraphicsView(this);
    _pGraphicsView->setObjectName("GraphicsView");
    _pGraphicsView->setScene(_pGraphicsScene);
    _pGraphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _pGraphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _pGraphicsView->setFrameShape(QFrame::NoFrame);
    _pGraphicsScene->installEventFilter(this);
    _pGraphicsView->installEventFilter(this);
    auto *loyout = new QGridLayout;
    loyout->setMargin(0);
    loyout->addWidget(_pGraphicsView);
    setLayout(loyout);
    connects();
}

PictureFrm::~PictureFrm()
{
    delete _pGraphicsView;
    delete _pGraphicsScene;
}

bool PictureFrm::loadNewPicture(const QString &picTure, bool isFirst)
{
    if (nullptr != _pPicItem)
    {
        _pGraphicsScene->removeItem(_pPicItem);
        delete _pPicItem;
    }

    _pPicItem = new PicItem(_scaleVal);
    _pGraphicsScene->addItem(_pPicItem);
    _strPicPath = picTure;
    _pixmap = QTalk::qimage::loadImage(picTure, false);

    if(_pixmap.isNull())
    {
        QtMessageBox::information(nullptr, tr("提示"), tr("加载图片失败"));
        _pPicBrowser->setVisible(false);
        return false;
    }

    double factor = 1.0;

    if (nullptr != _pPicBrowser)
    {
        double maxHeight = 0, maxWidth = 0;
        double picWidth = _pixmap.width();
        double picHeight = _pixmap.height();
        QRect deskRect;

        if(isFirst)
        {
            QWidget *wgt = UICom::getInstance()->getAcltiveMainWnd();
            QDesktopWidget *deskTop = QApplication::desktop();
            int curMonitor = deskTop->screenNumber(wgt);
            deskRect = deskTop->screenGeometry(curMonitor);
            maxWidth = deskRect.width() * 0.85;
            maxHeight = deskRect.height() * 0.85;
        }
        else
        {
            maxWidth = _pGraphicsView->width();
            maxHeight = _pGraphicsView->height();
        }

        double factor1 = maxWidth / picWidth;
        double factor2 = maxHeight / picHeight;

        if (factor1 < 1.0 || factor2 < 1.0)
            factor = qMin(factor1, factor2);

        picWidth = factor * picWidth;
        picHeight = factor * picHeight;
        _pGraphicsScene->setSceneRect(-picWidth / 2, -picHeight / 2, picWidth, picHeight);

        if(isFirst)
        {
            _pPicBrowser->resize(static_cast<int>(picWidth + 34), static_cast<int>(picHeight + 86));
            _pPicBrowser->move((deskRect.width() - _pPicBrowser->width()) / 2 + deskRect.x(),
                               (deskRect.height() - _pPicBrowser->height()) / 2 + deskRect.y());
        }
    }

    _scaleVal = 0;
    _angle = 0;
    _pPicItem->setPixmap(_pixmap, picTure, factor);
    return true;
}

//
void PictureFrm::connects()
{
    connect(_pPicBrowser, &PictureBrowser::enlargeSignal, this, [this]()
    {
        _scaleVal++;

        if(_pPicItem)
            _pPicItem->onScaleChange(_scaleVal, QPoint(0, 0));
    });
    connect(_pPicBrowser, &PictureBrowser::narrowSignal, this, [this]()
    {
        _scaleVal--;

        if(_pPicItem)
            _pPicItem->onScaleChange(_scaleVal, QPoint(0, 0));
    });
    connect(_pPicBrowser, &PictureBrowser::one2oneSiganl, this, [this]()
    {
        _scaleVal = 0;
        _pPicItem->setPos(0, 0);

        if(_pPicItem)
            _pPicItem->onScaleChange(_scaleVal, QPoint(0, 0));
    });
    connect(_pPicBrowser, &PictureBrowser::rotateSiganl, this, [this]()
    {
        _angle++;

        if(_pPicItem)
            _pPicItem->setRotation(_angle * -90);
    });
    connect(_pPicBrowser, &PictureBrowser::saveAsSignal, this, [this]()
    {
        QString strHistoryFileDir = QString::fromStdString(PLAT.getHistoryDir());
        QFileInfo oldFileInfo(_strPicPath);
        QString suffix = QTalk::qimage::getRealImageSuffix(_strPicPath).toLower();

        if(suffix.toLower() == "webp")
        {
            QString newPath = QFileDialog::getSaveFileName(this, tr("请选择文件保存路径"),
                              QString("%1/%2").arg(strHistoryFileDir, oldFileInfo.baseName()),
                              QString("(*.png);;(*.jpg);;(*.webp)"));

            if (!newPath.isEmpty())
            {
                PLAT.setHistoryDir(QFileInfo(newPath).absoluteDir().absolutePath().toStdString());
                QString newSuffix = QFileInfo(newPath).suffix().toUpper();
                //
                auto tmpPix = QTalk::qimage::loadImage(_strPicPath, false);

                if(!tmpPix.isNull())
                {
                    auto format = newSuffix.toUtf8().data();

                    if(tmpPix.save(newPath, format, 100))
                        QtMessageBox::information(_pPicBrowser, tr("提醒"), tr("图片保存成功"));
                }
            }
        }
        else
        {
            QString saveDir = QFileDialog::getSaveFileName(this, tr("请选择文件保存路径"),
                              QString("%1/%2").arg(strHistoryFileDir, oldFileInfo.baseName()),
                              QString("%1 (*.%1)").arg(suffix.isEmpty() ? "*" : suffix));

            if (!saveDir.isEmpty())
            {
                PLAT.setHistoryDir(QFileInfo(saveDir).absoluteDir().absolutePath().toStdString());
                QString newPath = QString("%1").arg(saveDir);

                if(QFileInfo(newPath).suffix().isEmpty() && !oldFileInfo.suffix().isEmpty())
                    newPath += QString(".%1").arg(oldFileInfo.suffix());

                if(QFile::copy(_strPicPath, newPath))
                    QtMessageBox::information(_pPicBrowser, tr("提醒"), tr("图片保存成功"));
            }
        }
    });
}

bool PictureFrm::eventFilter(QObject *o, QEvent *e)
{
//    static qint64 pt = 0;
    switch (e->type())
    {
//        case QEvent::TouchBegin:
//        case QEvent::TouchUpdate:
//        case QEvent::TouchEnd:
//        case QEvent::TouchCancel:
//        {
//            auto *touchEvent = dynamic_cast<QTouchEvent *>(e);
//            const QList<QTouchEvent::TouchPoint>& touchPoints = touchEvent->touchPoints();
//            if (touchPoints.count() == 2) {
//                switch (e->type()) {
//                    case QEvent::TouchBegin:
////                        touch = true;
//                        break;
//                    case QEvent::TouchUpdate:
//                    {
////                        touch = true;
//                        qint64 now = QDateTime::currentMSecsSinceEpoch();
//                        if(now - pt < 50)
//                            break;
//                        pt = now;
//                        const QTouchEvent::TouchPoint &pos0 = touchPoints.first();
//                        const QTouchEvent::TouchPoint &pos1 = touchPoints.last();
//                        qreal currentScaleFactor =
//                                QLineF(pos0.pos(), pos1.pos()).length()
//                                / QLineF(pos0.startPos(), pos1.startPos()).length();
//                        if(_pPicItem)
//                            _pPicItem->onScaleChange(currentScaleFactor > 1 ? ++_scaleVal : --_scaleVal,
//                                {0, 0});
//                        break;
//                    }
//                    case QEvent::TouchEnd:
//                    case QEvent::TouchCancel:
////                        touch = false;
//                        break;
//                    default:
//                        break;
//                }
//
//                break;
//            }
//        }
//#ifdef Q_OS_MAC
        case QEvent::NativeGesture:
            {
                auto *evt = dynamic_cast<QNativeGestureEvent *>(e);

                if(evt)
                {
                    switch (evt->gestureType())
                    {
                        case Qt::ZoomNativeGesture:
                            {
                                if (evt->value() > 0)
                                    _scaleVal++;
                                else
                                    _scaleVal--;

                                //
                                if(_pPicItem)
                                    _pPicItem->onScaleChange(_scaleVal, QPoint(0, 0));

                                break;
                            }

                        case Qt::SmartZoomNativeGesture:
                            {
                                // todo
//                        qInfo() << evt->value();
                                break;
                            }

                        case Qt::RotateNativeGesture:
                        case Qt::SwipeNativeGesture:
                        case Qt::PanNativeGesture:
                            break;

                        case Qt::BeginNativeGesture:
                        case Qt::EndNativeGesture:
                            break;
                    }
                }

                break;
            }

//#endif
//        case QEvent::Gesture:
//        {
//            qInfo() << "QEvent::Gesture";
//            break;
//        }
//        case QEvent::GestureOverride:
//        {
//            qInfo() << "QEvent::GestureOverride";
//            break;
//        }
        case QEvent::GraphicsSceneMousePress:
            {
                _startPos = QCursor::pos();
                _pressed = true;
                e->accept();
                break;
            }

        case QEvent::GraphicsSceneMouseRelease:
            {
                _pressed = false;
                e->accept();
                break;
            }

        case QEvent::GraphicsSceneMouseMove:
            {
                if (_pressed)
                {
                    QPoint p = QCursor::pos();

                    if(_pPicItem)
                        _pPicItem->moveBy((p.x() - _startPos.x()), (p.y() - _startPos.y()));

                    _startPos = p;
                    e->accept();
                }

                break;
            }

        case QEvent::GraphicsSceneWheel:
            {
//            qint64 now = QDateTime::currentMSecsSinceEpoch();
//            if(now - pt < 50)
//                break;
//            pt = now;
//            if(!touch)
//            {
                auto *evt = dynamic_cast<QGraphicsSceneWheelEvent *>(e);

                if(abs(evt->delta()) != 120)
                    break;

                if (evt->delta() > 0)
                    _scaleVal++;
                else
                    _scaleVal--;

                //
                if(_pPicItem)
                    _pPicItem->onScaleChange(_scaleVal, QPoint(0, 0));

//            e->accept();
//            }
                break;
            }

        case QEvent::Resize:
            {
                if(_pPicItem)
                    _pPicItem->setPos(0, 0);

                break;
            }

        case QEvent::KeyPress:
            {
                auto *evt = (QKeyEvent *)e;

                if(evt->key() == Qt::Key_Down || evt->key() == Qt::Key_Right)
                    _pPicBrowser->turnNext();
                else if(evt->key() == Qt::Key_Up || evt->key() == Qt::Key_Left)
                    _pPicBrowser->turnBefore();

                break;
            }

        default:
            break;
    }

    return QObject::eventFilter(o, e);
}

void PictureFrm::onCloseWnd()
{
    if (nullptr != _pPicItem)
    {
        _pGraphicsScene->removeItem(_pPicItem);
        delete _pPicItem;
        _pPicItem = nullptr;
    }
}
