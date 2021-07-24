//
// Created by cc on 2019-05-08.
//

#ifndef QTALK_V2_VOICEMESSAGEITEM_H
#define QTALK_V2_VOICEMESSAGEITEM_H

#include "MessageItemBase.h"
#include <QTimer>
#include <thread>

/**
* @description: VoiceMessageItem
* @author: cc
* @create: 2019-05-08 15:41
**/
class QHBoxLayout;
class QMediaPlayer;
class VoiceMessageItem : public MessageItemBase
{
    Q_OBJECT
public:
    explicit VoiceMessageItem(const StNetMessageResult &msgInfo, QWidget *parent = nullptr);
    ~VoiceMessageItem() override = default;;

public:
    QSize itemWdtSize() override;
    void stopVoice();

signals:
    void sgReleaseThread();

protected:
    void mousePressEvent(QMouseEvent *e) override;
    bool eventFilter(QObject *o, QEvent *e) override ;

private:
    void init();
    void initLayout();
    void initSendLayout();
    void initReceiveLayout();
    void initContentLayout();
    void releaseThread();

private:
    QSize _headPixSize;
    QMargins _mainMargin;
    QMargins _leftMargin;
    QMargins _rightMargin;
    QSize _contentSize;
    int _mainSpacing{};

    int _leftSpacing{};
    int _rightSpacing{};
    int _nameLabHeight{};

    QFrame       *_spaceFrm{nullptr};
    QHBoxLayout  *_mainLay{nullptr};
    QLabel       *_secondsLabel{nullptr};
    QTimer       *_pTimer{nullptr};
    HeadPhotoLab *_pixLabel{nullptr};

    std::string _localPath;

    std::thread *_thread{nullptr};
};

#endif //QTALK_V2_VOICEMESSAGEITEM_H
