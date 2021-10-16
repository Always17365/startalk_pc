//
// Created by cc on 2019-02-28.
//
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
#ifndef STALK_V2_SENDCODEWND_H
#define STALK_V2_SENDCODEWND_H

#include "CustomUi/UShadowWnd.h"
#include "CodeShell.h"
#include "CodeEdit.h"
#include "../../entity/UID.h"
#include "Util/Entity/JID.h"

/**
* @description: SendCodeWnd
* @author: cc
* @create: 2019-02-28 13:09
**/
class ChatViewItem;
class SendCodeWnd : public UShadowDialog{
	Q_OBJECT
public:
    explicit SendCodeWnd(QWidget* parent = nullptr);
    ~SendCodeWnd() override;

public:
    void addCode(const st::entity::UID& uid, const QString& code);

private:
    void initUi();
    void sendCode();

private:
    CodeShell* _pCodeShell;
    CodeEdit*  _pCodeEdit;

private:
    st::entity::UID  _uid;
};


#endif //STALK_V2_SENDCODEWND_H
