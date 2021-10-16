//
// Created by cc on 18-11-5.
//
#if _MSC_VER >= 1600
    #pragma execution_character_set("utf-8")
#endif
#ifndef STALK_V2_GROUPCARD_H
#define STALK_V2_GROUPCARD_H

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <memory>
#include <QMenu>
#include "CustomUi/TextEdit.h"
#include "CustomUi/UShadowWnd.h"
#include "entity/im_group.h"
#include "include/CommonStrcut.h"
#include "CustomUi/GroupMemberPopWnd.hpp"

class CardManager;
class HeadPhotoLab;
class ModButton;
class GroupCard : public UShadowDialog
{
    Q_OBJECT
public:
    explicit GroupCard(CardManager *cardManager);
    ~GroupCard() override;

public:
    void setData(std::shared_ptr<st::entity::ImGroupInfo> &data);
    void showGroupMember(const std::map<std::string, st::StUserCard> &,
                         const std::map<std::string, QUInt8> &userRole);
    void setForbiddenWordState(const QString &groupId, bool status);
    void setGroupId(const QString &groupId);

protected:
    bool eventFilter(QObject *o, QEvent *e) override;
    bool event(QEvent *e) override ;

private:
    void initUi();
    void sendMessageSlot();
    void onClose();
    void onSendMail();
    void onDestroyGroupGroupCard();
    void forbiddenWord();
    void checkAndEnableForbidden();

private:
    CardManager *_pCardManager{nullptr};
    QLabel      *_pNameLabel{nullptr};
    HeadPhotoLab *_pHeadLabel{nullptr};
    QPushButton *_pSendMailBtn{nullptr};
    QPushButton *_pExitGroupBtn{nullptr};
    QPushButton *_pDestroyGroupBtn{nullptr};
    QLineEdit   *_pGroupNameEdit{nullptr};
    QTextEdit    *_pGroupIdEdit{nullptr};
    QTextEdit    *_pGroupTopicEdit{nullptr};
    QLabel      *_pGroupMemberLabel{nullptr};
    QTextEdit    *_pGroupIntroduce{nullptr};
    QPushButton *_pSendMsgBtn{nullptr};
    QPushButton *_forbiddenBtn{nullptr};

    GroupMemberPopWnd *_pGroupMemberPopWnd{nullptr};

    ModButton    *_modGroupName{nullptr};
    ModButton    *_modGroupTopic{nullptr};
    ModButton    *_modGroupJJ{nullptr};

private:
    QString                  _strGroupId;
    QString                  _srtHead;
    QString                  _groupName;
    std::vector<std::string> groupMembers;

    bool _hasPermission;
    bool _forbiddenStatus{false};
    int _forbiddenFlag{0};

private:
    bool   _moded;
};

#endif //STALK_V2_GROUPCARD_H
