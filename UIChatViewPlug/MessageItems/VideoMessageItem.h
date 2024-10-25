//
// Created by admin on 2019-03-01.
//

#ifndef STALK_V2_VIDEOMESSAGEITEM_H
#define STALK_V2_VIDEOMESSAGEITEM_H

#include "MessageItemBase.h"
#include <QLabel>

class VideoMaskFrame;
class VideoMessageItem : public MessageItemBase {
	Q_OBJECT
public:
    explicit VideoMessageItem(const StNetMessageResult &msgInfo, QWidget *parent = nullptr);
    ~VideoMessageItem() override = default;;

public:
    QSize itemWdtSize() override;
    void setProcess(double speed, double dtotal, double dnow, double utotal, double unow);
    void downloadSuccess();

Q_SIGNALS:
    void sgPlayVideo(const QString &videoUrl, int width, int height);
    void sgDownloadedIcon(const QString&);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void setIcon(const QString& iconPath);
    void playVideo();

private:
    void init();

    void initLayout();

    void initSendLayout();

    void initReceiveLayout();

    void initContentLayout();

private:
    QLabel *_videoImgLab;
    QLabel *_playBtnLab;
    QLabel *_contentLab;
    VideoMaskFrame *maskFrame;
//    QLabel *btnLable;

    QSize _headPixSize;
    QMargins _mainMargin;
    QMargins _leftMargin;
    QMargins _rightMargin;
    QSize _contentSize;
    int _mainSpacing;

    int _leftSpacing;
    int _rightSpacing;
    int _nameLabHeight;

    QSize contentSize;
    QString localVideo;
};


#endif //STALK_V2_VIDEOMESSAGEITEM_H
