//
// Created by cc on 2020/12/22.
//

#ifndef STALK_V2_GROUPWND_H
#define STALK_V2_GROUPWND_H

#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>

#include "entity/im_group.h"
#include "include/CommonStrcut.h"
#include "CustomUi/HeadPhotoLab.h"
#include "CustomUi/ModButton.h"
#include "CustomUi/GroupMemberPopWnd.hpp"

class AddressBookPanel;
class GroupWnd : public QFrame
{
    Q_OBJECT

public:
    explicit GroupWnd(AddressBookPanel *parent = nullptr);

Q_SIGNALS:
    void sgUpdateGroupMember(std::map<std::string, st::StUserCard>, std::map<std::string, QUInt8> userRole);

public:
    void setData(std::shared_ptr<st::entity::ImGroupInfo> &data);

private:
    void sendMessageSlot();
    void showGroupMember(const std::map<std::string, st::StUserCard> &, const std::map<std::string, QUInt8> &userRole);
    void onSendMail();
    void onDestroyGroupGroupCard();

private:
    AddressBookPanel *_mainPanel{};

    QLabel *_pNameLabel{};
    HeadPhotoLab *_pHeadLabel{};
    QPushButton *_pSendMailBtn{};
    QPushButton *_pExitGroupBtn{};
    QPushButton *_pDestroyGroupBtn{};
    QLineEdit *_pGroupNameEdit{};
    QTextEdit *_pGroupIdEdit{};
    QTextEdit *_pGroupTopicEdit{};
    QLabel *_pGroupMemberLabel{};
    QTextEdit *_pGroupIntroduce{};
    QPushButton *_pSendMsgBtn{};

    GroupMemberPopWnd *_pGroupMemberPopWnd{};
    QString _strGroupId;
    QString _groupName;
    QString _srtHead;

    std::vector<std::string> groupMembers;
};

#endif //STALK_V2_GROUPWND_H
