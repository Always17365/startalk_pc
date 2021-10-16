#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
#ifndef MESSAGEITEMBASE_H
#define MESSAGEITEMBASE_H

#include <QFrame>
#include <QMenu>
#include <QToolButton>
#include <QLabel>
#include "../NativeChatStruct.h"
#include "../../entity/im_message.h"
#include "DataCenter/dbPlatForm.h"
#include "CustomUi/MedalWgt.h"

#define HEAD_RADIUS 14

/**
  * @函数名
  * @功能描述 所有消息类基类
  * @参数
  * @author
  * @date 2018.10.15
  */
class HeadPhotoLab;
class ChatViewItem;
class MessageItemBase : public QFrame
{
    Q_OBJECT

public:
    explicit MessageItemBase(const StNetMessageResult &msgInfo, QWidget *parent = nullptr);
    ~MessageItemBase() override;

public:
    virtual QSize itemWdtSize() { return {0,0};};
    inline int messageType() { return _msgInfo.msg_type; };
    void updateMessageInfo(const StNetMessageResult& messageInfo);

public:
    void setReadState(const QInt32& state);
    bool contains(const QPoint& pos);
    void showShareCheckBtn(bool show);
    void checkShareCheckBtn(bool check);
    void onDisconnected();
    void updateUserMedal();

Q_SIGNALS:
    void sgSelectItem(bool);
    void sgDisconnected();
    void sgUpdateUserMedal();

protected:
    bool eventFilter(QObject *o, QEvent *e) override;
    bool event(QEvent* e) override;

public:
    StNetMessageResult _msgInfo;

protected:
    QInt32 _readSts;

protected:
    HeadPhotoLab *_headLab{};
    QLabel *_nameLab{};
    MedalWgt* _medalWgt{};
    QFrame *_contentFrm{};
    QLabel *_readStateLabel{};
    QToolButton* _btnShareCheck{};

    HeadPhotoLab *_sending{};
    HeadPhotoLab *_resending{};

private:
    // bool _isPressEvent{};
    // bool _alreadyRelease{};

public:
    enum readState
    {
        EM_BLACK_RET = -1,
        EM_SENDDING = 0,
        EM_READSTS_UNREAD = 1,
        EM_READSTS_READED = 2,
    };
};

#endif // MESSAGEITEMBASE_H
