//
// Created by cc on 2018/11/16.
//

#ifndef STALK_V2_PICTUREFRM_H
#define STALK_V2_PICTUREFRM_H

#include <QFrame>
#include <QGraphicsScene>
#include <QGraphicsView>
#include "PicItem.h"

class PictureBrowser;

class PictureFrm : public QFrame {
Q_OBJECT
public:
    explicit PictureFrm(PictureBrowser *pBrowser);

    ~PictureFrm() override;

public:
    bool loadNewPicture(const QString &picTure, bool isFirst);
    void onCloseWnd();

protected:
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    void connects();

private:
    PictureBrowser *_pPicBrowser;
    QGraphicsScene *_pGraphicsScene;
    QGraphicsView *_pGraphicsView;
    PicItem *_pPicItem;

    QPixmap _pixmap;

private:
    int _scaleVal;
    int _angle;
    QString _strPicPath;

private:
    QPointF _startPos;
    bool _pressed;
};


#endif //STALK_V2_PICTUREFRM_H
