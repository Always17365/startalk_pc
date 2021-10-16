#include "ChatViewMainPanel.h"
#include <QStackedLayout>
#include <QDateTime>
#include <QApplication>
#include <QPixmap>
#include <QClipboard>
#include <QMovie>
#include <QSplitter>
#include <QBuffer>
#include <QMimeData>
#include <QDesktopWidget>
#include <QMediaPlayer>
#include <QTemporaryFile>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QtConcurrent>
#include <math.h>

#include "ChatMainWgt.h"
#include "StatusWgt.h"
#include "InputWgt.h"
#include "GroupChatSidebar.h"
#include "DataCenter/Platform.h"
#include "Util/Entity/JID.h"
#include "GroupMember.h"
#include "GroupTopic.h"
#include "Emoticon/EmoticonMainWgt.h"
#include "Util/nJson/nJson.h"
#include "MessagePrompt.h"
#include "../quazip/JlCompress.h"
#include "DataCenter/dbPlatForm.h"
#include "CustomUi/LiteMessageBox.h"
#include "Util/ui/qimage/qimage.h"
#include "DataCenter/AppSetting.h"
#include "code/SendCodeWnd.h"
#include "code/CodeShowWnd.h"
#include "CustomUi/QtMessageBox.h"
#include "ShareMessageFrm.h"
#include "DataCenter/NavigationManager.h"
#include "SelectUserWnd.h"
#include "MessageItems/VoiceMessageItem.h"
#include "MessageItems/CodeItem.h"
#include "QRcode/ScanQRcode.h"
#include "search/LocalSearchMainWgt.h"
#include "ChatUtil.h"
#include "MessageItems/FileSendReceiveMessItem.h"
#include "MessageItems/VideoMessageItem.h"
#include "Util/utils.h"
#include "Util/Log.h"

#ifdef Q_OS_MAC
    #include "sound/PlayAudioSound.h"
    #include "MacNotification.h"
    #include "../Screenshot/mac/SnipScreenTool.h"
#else
    #include "../Screenshot/SnipScreenTool.h"
#endif

#ifdef Q_OS_WIN
    #include <wrapper.h>
    #include <QAudioOutput>
    #include <QSharedPointer>
    #include <QMediaPlayer>
    #include "sound/amr/VoiceHelper.h"

    QSharedPointer<WavOutput> spOutputDevice;
    QSharedPointer<QAudioOutput> spAudioOutput;
#endif // _WINDOWS
#include "WebService/WebService.h"

//
using namespace st;
using namespace st::entity;
using std::string;
using std::map;

extern ChatViewMainPanel *g_pMainPanel{nullptr};

void deleteViewItem(const st::entity::UID &, ChatViewItem *viewItem)
{
    viewItem->deleteLater();
}

ChatViewMainPanel::ChatViewMainPanel(QWidget *parent) : QFrame(parent),
    _strSelfId(""),
    _mapItems(200, deleteViewItem)
{
    g_pMainPanel = this;
    setObjectName("ChatViewMainPanel");
    _netManager = new QNetworkAccessManager(this);
    _downloadImageThread.start();
    _pSelectUserWnd = new SelectUserWnd(this);
    _pQRcode = new QRcode(this);
    //
    _pStackedLayout = new QStackedLayout(this);
    showEmptyLabel();
    // 拖拽发送文件
    setAcceptDrops(true);
    //
    new ChatMsgListener;
    gifManager = new GIFManager;
    //
    QString fileConfigPath = QString("%1/fileMsgPath").arg(
                                 DC.getConfigPath().data());
    _pFileMsgConfig = new st::ConfigLoader(fileConfigPath.toLocal8Bit().data());
    _pFileMsgConfig->load();
    // 初始化提示音
    initSound();
    // 加载截屏 表情插件
    _pSnipScreenTool = SnipScreenTool::getInstance();
    initPlug();
    //
    _pAudioVideoManager = new AudioVideoManager;
    connect(_pAudioVideoManager, &AudioVideoManager::sgSendSignal, this,
            &ChatViewMainPanel::onSendSignal);
    connect(_pAudioVideoManager, &AudioVideoManager::sgClose2AudioVideo,
    [this](const QString & peerId, bool isVideo, long long occupied_time) {
        QString json =
            QString("{\"type\":\"close\", \"time\": %1, \"desc\":\"\" }").arg(
                occupied_time);
        sendAudioVideoMessage(st::entity::UID(peerId), st::Enum::TwoPersonChat,
                              isVideo, json);
    });
    //
    qRegisterMetaType<st::entity::ImMessageInfo>("st::entity::ImMessageInfo");
    qRegisterMetaType<string>("string");
    qRegisterMetaType<std::vector<st::StUserCard>>("std::vector<st::StUserCard>");
    connect(this, &ChatViewMainPanel::showMessagePromptSignal, this,
            &ChatViewMainPanel::showPrompt);
    connect(this, &ChatViewMainPanel::sgUpdateGroupMember, this,
            &ChatViewMainPanel::updateGroupMemberInfo, Qt::QueuedConnection);
    connect(_pSnipScreenTool, &SnipScreenTool::sgFinish, this,
            &ChatViewMainPanel::onSnapFinish);
    connect(_pSnipScreenTool, &SnipScreenTool::sgCancel, this,
            &ChatViewMainPanel::onSnapCancel);
    connect(this, &ChatViewMainPanel::sgReloadChatView, this,
            &ChatViewMainPanel::reloadChatView);
    connect(this, &ChatViewMainPanel::sgDisconnected, this,
            &ChatViewMainPanel::onDisConnected);
    connect(this, &ChatViewMainPanel::sgShowLiteMessageBox, this,
            &ChatViewMainPanel::showLiteMessageBox);
    connect(this, &ChatViewMainPanel::sgShowInfoMessageBox, this,
            &ChatViewMainPanel::showInfoMessageBox);
    connect(this, &ChatViewMainPanel::sgPlaySound, this,
            &ChatViewMainPanel::playSound);
    connect(this, &ChatViewMainPanel::sgShowVidioWnd, this,
            &ChatViewMainPanel::onShowVideoWnd);
    connect(this, &ChatViewMainPanel::showQRcode, this,
            &ChatViewMainPanel::onShowQRcodeWnd);
    connect(EmoticonMainWgt::instance(), &EmoticonMainWgt::sendEmoticon, this,
            &ChatViewMainPanel::sendEmoticonMessage);
    connect(EmoticonMainWgt::instance(), &EmoticonMainWgt::sgInsertEmoticon,
            [this](const QString & id, const QString & pkgId, const QString & shortCut,
    const QString & localPath) {
        if (_pViewItem && id != _pViewItem->conversionId()) {
            return;
        }

        if (_pViewItem && _pViewItem->_pInputWgt) {
            _pViewItem->_pInputWgt->insertEmotion(pkgId, shortCut, localPath);
            return;
        }
    });
    connect(EmoticonMainWgt::instance(), &EmoticonMainWgt::sendCollectionImage,
            this,
            &ChatViewMainPanel::sendCollectionMessage);
    //
    QPointer<ChatViewMainPanel> pThis(this);
    QT_CONCURRENT_FUNC([pThis]() {
        if (pThis) {
            std::vector<st::StShareSession> ss;
            ChatMsgManager::getContactsSession(ss);

            if (!pThis) {
                return;
            }

            if (pThis && !ss.empty()) {
                pThis->_pSelectUserWnd->setSessions(SelectUserWnd::EM_TYPE_CONTACTS, ss);
            }
        }
    });
}

ChatViewMainPanel::~ChatViewMainPanel()
{
#ifdef Q_OS_WIN

    for (MessagePrompt *msgPrompt : _arMessagePrompt) {
        delete msgPrompt;
        msgPrompt = nullptr;
    }

#endif
    WebService::releaseService();
    _downloadImageThread.quit();
    _downloadImageThread.wait(2000);
}

/**
  * @函数名   onChatUserChanged
  * @功能描述 接受切换聊天会话请求
  * @参数
  * @author   cc
  * @date     2018/09/19
  */
void ChatViewMainPanel::onChatUserChanged(const StSessionInfo &info)
{
    if (nullptr != _pStackedLayout) {
        UID uid = UID(info.userId, info.realJid);

        if (_pViewItem && _pViewItem->_uid == uid) {
            _pViewItem->_pInputWgt->setFocus();
            return;
        }

        if (_pViewItem
            && _pViewItem->_uid.qUsrId().section("@", 0, 0) != SYSTEM_XMPPID) {
            QString content = _pViewItem->_pInputWgt->translateText();

            if (!content.isEmpty()) {
                emit sgShowDraft(_pViewItem->_uid, content);
                _his_input_data[_pViewItem->_uid] = content;
            }

            _pViewItem->freeView();
        }

        // 先进入聊天窗口再拉历史消息
        auto func = [this, info, uid]() {
            // 切换主画面
            ChatViewItem *item = _pViewItem;

            if (nullptr != item) {
                if (item->_pInputWgt->isVisible()) {
                    item->_pInputWgt->setFocus();
                } else {
                    item->setFocus();
                }

                QString conversionId = item->conversionId();
                _pSnipScreenTool->setConversionId(conversionId);
            }

            // 连接状态
            if (item && item->_pChatMainWgt) {
                item->_pChatMainWgt->setConnectState(_isConnected);
            }
        };
        int chatType = info.chatType;

        //
        if (_mapItems.contains(uid)) {
            _pViewItem = _mapItems.get(uid);
            _mapItems.update(uid);
            _pStackedLayout->setCurrentWidget(_pViewItem);
            func();
        } else {
            _pViewItem = new ChatViewItem();
            _mapItems.insert(uid, _pViewItem);
            _pStackedLayout->addWidget(_pViewItem);
            _pViewItem->switchSession(chatType, info.userName, uid);

            // 草稿
            if (_his_input_data.contains(uid)) {
                if (_pViewItem && _pViewItem->_pInputWgt) {
                    _pViewItem->_pInputWgt->setContentByJson(_his_input_data[uid]);
                }

                _his_input_data.remove(uid);
            }

            _pStackedLayout->setCurrentWidget(_pViewItem);
            func();
            auto functor = QT_CONCURRENT_FUNC([this, info, uid, chatType]() {
                // 获取群信息
                if (chatType == Enum::GroupChat) {
                    ChatMsgManager::getGroupInfo(info.userId.toStdString());
                    // ChatMsgManager::getforbiddenWordState(info.userId.toStdString());
                }

                //
                std::vector<st::entity::ImMessageInfo> msgLst =
                    ChatMsgManager::getUserHistoryMessage(0, info.chatType, uid);
                auto it = msgLst.begin();

                for (; it != msgLst.end(); it++) {
                    st::entity::ImMessageInfo msg = *it;

                    if (SYSTEM_XMPPID == st::entity::JID(it->XmppId).username()) {
                        msg.HeadSrc = ":/chatview/image1/system.png";
                        _mapHeadPath[QString::fromStdString(it->XmppId)] = QString::fromStdString(
                                                                               msg.HeadSrc);
                    } else {
                        if (it->ChatType == Enum::GroupChat) {
                            if (!it->HeadSrc.empty()) {
                                auto localPath = st::GetHeadPathByUrl(it->HeadSrc);
                                msg.HeadSrc = localPath;
                            }

                            msg.UserName = st::getUserName(msg.From);
                        } else {
                            msg.HeadSrc = info.headPhoto.toStdString();
                        }

                        if (!msg.HeadSrc.empty()) {
                            QMutexLocker locker(&_mutex);
                            _mapHeadPath[QString::fromStdString(it->From)] = QString::fromStdString(
                                                                                 msg.HeadSrc);
                        }
                    }

                    if (_pViewItem) {
                        _pViewItem->showMessage(msg, ChatMainWgt::EM_JUM_BOTTOM);
                    }
                }
            });
        }

        if (_pViewItem && _pViewItem->_pInputWgt
            && _pViewItem->_pInputWgt->isVisible()) {
            _pViewItem->_pInputWgt->setFocus();
        }
    }
}

/**
  * @函数名   onRecvMessage
  * @功能描述
  * @参数
  * @author   cc
  * @date     2018/09/20
  */
void ChatViewMainPanel::onRecvMessage(R_Message &e, bool isCarbon)
{
    st::entity::ImMessageInfo message = e.message;
    entity::JID jid(message.SendJid);
    entity::JID from(message.From);
    string userId = jid.basename();
    string realJid = message.RealJid.empty() ? userId : message.RealJid;
    string realFrom = st::entity::JID(message.RealFrom).basename();
    UID uid(userId, realJid);
    message.XmppId = userId;
    message.RealJid = realJid;
    {
        QMutexLocker locker(&_mutex);

        if (from.basename() != getSelfUserId()) {
            message.Direction = st::entity::MessageDirectionReceive;
            static QInt64 recvMessageTime = 0;
            QInt64 currentTime = QDateTime::currentMSecsSinceEpoch();

            if (currentTime - recvMessageTime >= 1000 &&
                _mapNotice.find(userId) == _mapNotice.end()) {
                if (message.UserName.empty()) {
                    auto info = DB_PLAT.getUserInfo(realFrom);

                    if (nullptr == info) {
                        info = std::make_shared<st::entity::ImUserInfo>();
                        info->XmppId = realFrom;
                        ChatMsgManager::getUserInfo(info);
                    }

                    message.UserName = st::getUserName(info);
                }

                recvMessageTime = currentTime;
                // 消息提示
                emit recvMessageSignal();

                // 消息提示音
                if (AppSetting::instance().getNewMsgAudioNotify()) {
                    emit sgPlaySound();
                }

                // 消息提示窗
                if (AppSetting::instance().getNewMsgTipWindowNotify()) {
                    emit showMessagePromptSignal(message);
                }
            }
        } else {
            message.Direction = st::entity::MessageDirectionSent;
            emit sgUserSendMessage();
        }

        if (message.Type == st::entity::MessageTypeShock && !isCarbon) {
            emit sgWakeUpWindow();
        }

        if (!message.AutoReply && message.ChatType == st::Enum::TwoPersonChat
            && from.basename() != getSelfUserId()) {
            sendAutoPeplyMessage(uid, message.MsgId);
        }

        if (!_mapItems.contains(uid)) {
            debug_log("not show message from {0} messageId:{1}", uid.toStdString(),
                      message.MsgId);
            return;
        }

        auto *pViewItem = _mapItems.get(uid);

        if (nullptr == pViewItem) {
            return;
        }

        st::entity::JID jidSender(message.From);
        QString sender = QString::fromStdString(jidSender.basename());

        if (_mapHeadPath.contains(sender)) {
            message.HeadSrc = _mapHeadPath[sender].toStdString();
        } else if (SYSTEM_XMPPID == st::entity::JID(message.XmppId).username()) {
            message.HeadSrc = ":/chatview/image1/system.png";
            _mapHeadPath[QString::fromStdString(message.XmppId)] = QString::fromStdString(
                                                                       message.HeadSrc);
        }

        if (message.ChatType == st::Enum::GroupChat) {
            message.UserName = st::getUserName(realFrom);
        }

        if (pViewItem)
            pViewItem->showMessage(message,
                                   message.Direction == st::entity::MessageDirectionSent
                                   ? ChatMainWgt::EM_JUM_BOTTOM
                                   : ChatMainWgt::EM_JUM_INVALID);
    }
}

/**
  * @函数名
  * @功能描述
  * @参数
  * @author   cc
  * @date     2018/10/19
  */
void ChatViewMainPanel::onRecvFileProcessMessage(const FileProcessMessage &e)
{
    // 只刷新当前会话
    if (_pViewItem && _pViewItem->_pChatMainWgt) {
        _pViewItem->_pChatMainWgt->recvFileProcess(e.speed, e.downloadTotal,
                                                   e.downloadNow, e.uploadTotal,
                                                   e.uploadNow, e.key);
    }
}

/**
  * @函数名   getSelfUserId
  * @功能描述 获取自身ID
  * @参数
  * @author   cc
  * @date     2018/09/20
  */
string ChatViewMainPanel::getSelfUserId()
{
    if (_strSelfId.empty()) {
        _strSelfId = DC.getSelfXmppId();
    }

    return _strSelfId;
}

/**
  * @函数名   updateGroupMember
  * @功能描述 更新群列表
  * @参数
  * @author   cc
  * @date     2018/10/08
  */
void ChatViewMainPanel::updateGroupMember(const GroupMemberMessage &e)
{
    UID uid(e.groupId);

    if (!_mapItems.contains(uid)) {
        return;
    }

    auto *pViewItem = _mapItems.get(uid);

    if (pViewItem) {
        if (pViewItem && pViewItem->_pGroupSidebar) {
            pViewItem->_pGroupSidebar->updateGroupMember(e);
        }

        if (pViewItem && pViewItem->_pInputWgt) {
            pViewItem->_pInputWgt->updateGroupMember(e.members);
        }
    }
}

/**
  * @函数名   updateHead
  * @功能描述 更新头像
  * @参数
  * @author   cc
  * @date     2018/10/09
  */
void ChatViewMainPanel::updateHead()
{
    for (const auto &pItem : _mapItems.values()) {
        if (pItem && pItem->_chatType == st::Enum::GroupChat &&
            pItem->_pGroupSidebar && pItem->_pGroupSidebar->_pGroupMember) {
            pItem->_pGroupSidebar->_pGroupMember->updateHead();
        }
    }
}

/**
  * @函数名
  * @功能描述
  * @参数
  * @author   cc
  * @date     2018/10/09
  */
void ChatViewMainPanel::updateUserHeadPath(const std::vector<st::StUserCard>
                                           &users)
{
    for (const auto &user : users) {
        {
            QMutexLocker locker(&_mutex);
            string headSrc = st::GetHeadPathByUrl(user.headerSrc);
            _mapHeadPath[QString(user.xmppId.c_str())] = headSrc.c_str();
        }
        UID uid(user.xmppId);

        if (_mapItems.contains(uid)) {
            auto *pItem = _mapItems.get(uid);

            if (pItem && pItem->_pStatusWgt) {
                emit pItem->_pStatusWgt->updateName(st::getUserName(user.xmppId).data());
            }
        }
    }
}

/**
  * @函数名   updateGroupTopic
  * @功能描述 更新群公告
  * @参数
  * @author   cc
  * @date     2018/10/12
  */
void ChatViewMainPanel::updateGroupTopic(const string &groupId,
                                         const string &topic)
{
    UID uid(groupId);

    if (!_mapItems.contains(uid)) {
        return;
    }

    auto *pViewItem = _mapItems.get(uid);

    if (pViewItem && pViewItem->_pGroupSidebar) {
        emit pViewItem->_pGroupSidebar->updateGroupTopic(topic.c_str());
    }
}

/**
  * @函数名
  * @功能描述
  * @参数
  * @author   cc
  * @date     2018/10/26
  */
void ChatViewMainPanel::updateGroupMemberInfo(const string &groupId,
                                              const std::vector<st::StUserCard> &userCards)
{
    if (!DC.isMainThread()) {
        emit sgUpdateGroupMember(groupId, userCards);
        return;
    }

    UID uid(groupId);

    if (!_mapItems.contains(uid)) {
        return;
    }

    auto *pViewItem = _mapItems.get(uid);

    if (pViewItem) {
        pViewItem->updateGroupMember(userCards);


        updateUserHeadPath(userCards);
    }
}

/**
  * @函数名   setConnectStatus
  * @功能描述 与服务器连接状态
  * @参数
  * @author   cc
  * @date     2018/10/25
  */
void ChatViewMainPanel::setConnectStatus(bool isConnect)
{
    QMutexLocker locker(&_mutex);
    _isConnected = isConnect;

    if (_isConnected) {
        emit sgReloadChatView();
    } else {
        emit sgDisconnected();
    }
}

/**
  * @函数名   getConnectStatus
  * @功能描述 获取与服务器连接状态
  * @参数
     bool 连接状态
  * @author   cc
  * @date     2018/10/25
  */
bool ChatViewMainPanel::getConnectStatus()
{
    return _isConnected;
}

/**
  * @函数名
  * @功能描述 加载截屏 表情插件
  * @参数
  * @author   cc
  * @date     2018/10/30
  */
void ChatViewMainPanel::initPlug()
{
    //
    QString styleSheetPath = QString(":/style/style1/Screenshot.qss");

    if (QFile::exists(styleSheetPath)) {
        QFile file(styleSheetPath);

        if (file.open(QFile::ReadOnly)) {
            QString qss = QString::fromUtf8(file.readAll()); //以utf-8形式读取文件
            _pSnipScreenTool->setToolbarStyle(qss.toUtf8());
            file.close();
        }
    }
}

//
void ChatViewMainPanel::showUserCard(const QString &userId)
{
    emit showUserCardSignal(userId);
}

/**
 *
 */
void ChatViewMainPanel::getHistoryMsg(const QInt64 &t, const QUInt8 &chatType,
                                      const st::entity::UID &uid)
{
    //
    if (DC.isMainThread()) {
        //
        static qint64 req_time = 0;
        qint64 cur_time = QDateTime::currentMSecsSinceEpoch();

        if (cur_time - req_time < 500) {
            return;
        }

        req_time = cur_time;
        QT_CONCURRENT_FUNC([this, t, chatType, uid] { getHistoryMsg(t, chatType, uid); });
        return;
    }

    if (!_mapItems.contains(uid)) {
        return;
    }

    auto *pViewItem = _mapItems.get(uid);

    if (pViewItem) {
        emit pViewItem->sgLoadingMovie(true);
    }

    std::vector<st::entity::ImMessageInfo> msgLst =
        ChatMsgManager::getUserHistoryMessage(t, chatType, uid);

    //
    if (msgLst.empty() && pViewItem && pViewItem->_pChatMainWgt) {
        pViewItem->_pChatMainWgt->setHasNotHistoryMsgFlag(false);
    }

    for (auto &it : msgLst) {
        if (SYSTEM_XMPPID == st::entity::JID(it.XmppId).username()) {
            it.HeadSrc = ":/chatview/image1/system.png";
            _mapHeadPath[uid.qUsrId()] = QString::fromStdString(it.HeadSrc);
        } else {
            if (it.ChatType == Enum::GroupChat) {
                if (!it.HeadSrc.empty()) {
                    it.HeadSrc = st::GetHeadPathByUrl(it.HeadSrc);
                }

                it.UserName = st::getUserName(it.From);
            } else {
                if (_mapHeadPath.contains(uid.qReadJid())) {
                    it.HeadSrc = _mapHeadPath[uid.qReadJid()].toStdString();
                }
            }
        }

        if (!it.HeadSrc.empty()) {
            _mapHeadPath[uid.qReadJid()] = QString::fromStdString(it.HeadSrc);
        }

        if (pViewItem) {
            pViewItem->showMessage(it, ChatMainWgt::EM_JUM_ITEM);
        } else {
            return;
        }
    }

    if (pViewItem) {
        emit pViewItem->sgLoadingMovie(false);
    }
}

//
void ChatViewMainPanel::gotReadState(const UID &uid,
                                     const map<string, QInt32> &msgMasks)
{
    if (!_mapItems.contains(uid)) {
        return;
    }

    auto *pViewItem = _mapItems.get(uid);

    if (pViewItem) {
        emit pViewItem->_pChatMainWgt->gotReadStatueSignal(msgMasks);
    }
}

//
void ChatViewMainPanel::gotMState(const st::entity::UID &uid,
                                  const string &messageId, const QInt64 &time)
{
    //
    if (!_mapItems.contains(uid)) {
        return;
    }

    auto *pViewItem = _mapItems.get(uid);

    if (pViewItem && pViewItem->_pChatMainWgt) {
        emit pViewItem->_pChatMainWgt->sgGotMState(messageId.data(), time);
    }
}

//
void ChatViewMainPanel::updateRevokeMessage(const st::entity::UID &uid,
                                            const string &fromId, const string &messageId, const long long time)
{
    if (!_mapItems.contains(uid)) {
        return;
    }

    auto *pViewItem = _mapItems.get(uid);

    if (pViewItem && pViewItem->_pChatMainWgt) {
        emit pViewItem->_pChatMainWgt->updateRevokeSignal(QString::fromStdString(
                                                              fromId),
                                                          QString::fromStdString(messageId), time);
    }
}

/**
  * @函数名
  * @功能描述
  * @参数
  * @author   cc
  * @date     2018/12/13
  */
void ChatViewMainPanel::onDestroyGroup(const string &groupId,
                                       bool isDestroy)
{
    st::entity::UID uid(groupId);
    //    if (_pViewItem && _pViewItem->_uid == uid) {
    //        _pStackedLayout->setCurrentWidget(_pEmptyLabel);
    //    }
    //
    QString name = st::getGroupName(groupId).data();

    if (name.isEmpty()) {
        name = QString::fromStdString(groupId).section("@", 0, 0);
    }

    emit sgShowLiteMessageBox(true,
                              isDestroy ? QString(tr("群 \"%1\" 已被销毁")).arg(name) : QString(
                                  tr("您已退出/被移出群聊 \"%1\"")).arg(name));
}

QString dealMessageContent(const st::entity::ImMessageInfo &msg)
{
    QString ret = "";

    if (msg.ChatType == st::Enum::GroupChat && !msg.UserName.empty()) {
        ret += QString(msg.UserName.data()) + ": ";
    }

    switch (msg.Type) {
    case st::entity::MessageTypeFile:
        ret += QObject::tr("[文件]");
        break;

    case st::entity::MessageTypePhoto:
        ret += QObject::tr("[图片]");
        break;

    case st::entity::MessageTypeImageNew:
        ret += QObject::tr("[表情]");
        break;

    case st::entity::MessageTypeSmallVideo:
        ret += QObject::tr("[视频]");
        break;

    case st::entity::WebRTC_MsgType_VideoCall:
        ret += QObject::tr("[实时视频]");
        break;

    case st::entity::WebRTC_MsgType_AudioCall:
        ret += QObject::tr("[实时音频]");
        break;

    case st::entity::WebRTC_MsgType_Video_Group:
        ret += QObject::tr("[群组视频]");
        break;

    case st::entity::MessageTypeCommonTrdInfo:
    case st::entity::MessageTypeCommonTrdInfoV2:
        ret += QObject::tr("[链接卡片]");
        break;

    case st::entity::MessageTypeSourceCode:
        ret += QObject::tr("[代码块]");
        break;

    case st::entity::MessageTypeVoice:
        ret += QObject::tr("[语音]");
        break;

    case 65537:
    case 65538:
        ret += QObject::tr("[热线提示信息]");
        break;

    default:
    case st::entity::MessageTypeText:
    case st::entity::MessageTypeGroupAt:
    case st::entity::MessageTypeRobotAnswer: {
        QString tmpContent = QString::fromStdString(msg.Content).split("\n").first();
        QRegExp regExp("\\[obj type=[\\\\]?\"([^\"]*)[\\\\]?\" value=[\\\\]?\"([^\"]*)[\\\\]?\"(.*)\\]");
        regExp.setMinimal(true);
        int pos = 0;

        while ((pos = regExp.indexIn(tmpContent)) != -1) {
            QString item = regExp.cap(0); // 符合条件的整个字符串
            QString type = regExp.cap(1); // 多媒体类型

            if ("url" == type) {
                tmpContent.replace(pos, item.size(), QObject::tr("[链接]"));
            } else if ("image" == type) {
                tmpContent.replace(pos, item.size(), QObject::tr("[图片]"));
            } else if ("emoticon" == type) {
                tmpContent = QObject::tr("[表情]");
            } else {
                tmpContent.replace(pos, item.size(), QObject::tr("[未知类型]"));
            }
        }

        ret += tmpContent;
        break;
    }
    }

    return ret;
}

void getParam(const st::entity::ImMessageInfo &msg,
              StNotificationParam &param)
{
    //
    st::entity::JID jid(msg.SendJid);
    string headPath;

    if (msg.ChatType == st::Enum::GroupChat) {
        std::shared_ptr<st::entity::ImGroupInfo> groupInfo = DB_PLAT.getGroupInfo(
                                                                 jid.basename(), true);

        if (nullptr != groupInfo) {
            param.title = groupInfo->Name;
            headPath = groupInfo->HeaderSrc;
        }

        if (!headPath.empty()) {
            headPath = st::GetHeadPathByUrl(headPath);
        }

        if (headPath.empty() || !QFile::exists(headPath.data())) {
            headPath = ":/QTalk/image1/StarTalk_defaultGroup.png";
        }
    } else if (msg.ChatType == st::Enum::System) {
        headPath = ":/chatview/image1/system.png";
        param.title = QObject::tr("系统消息").toStdString();
    } else {
        std::shared_ptr<st::entity::ImUserInfo> userInfo = DB_PLAT.getUserInfo(
                                                               jid.basename());

        if (nullptr != userInfo) {
            param.title = st::getUserName(userInfo);
            headPath = userInfo->HeaderSrc;
        }

        if (!headPath.empty()) {
            headPath = st::GetHeadPathByUrl(headPath);
        }

        if (headPath.empty() || !QFile::exists(headPath.data())) {
            headPath = ":/QTalk/image1/StarTalk_defaultHead.png";
        }
    }

    param.icon = headPath;
    param.message = dealMessageContent(msg).toStdString();
    param.playSound = AppSetting::instance().getNewMsgAudioNotify();
    param.xmppId = entity::JID(msg.SendJid).basename();
    param.from = entity::JID(msg.From).basename();
    param.realJid = entity::JID(msg.RealJid).basename();
    param.chatType = msg.ChatType;
}

// 显示提示消息
void ChatViewMainPanel::showPrompt(const st::entity::ImMessageInfo &msg)
{
    StNotificationParam param;
    getParam(msg, param);
#if defined(_MACOS)
    param.loginUser = getSelfUserId();
    st::mac::Notification::showNotification(&param);
#else

    if (AppSetting::instance().getUseNativeMessagePrompt()) {
        emit sgShowNotify(param);
        return;
    }

    //
    auto *prompt = new MessagePrompt(msg);
    prompt->setAttribute(Qt::WA_QuitOnClose, false);
    connect(prompt, &MessagePrompt::openChatWnd, this,
            &ChatViewMainPanel::onShowChatWnd);
    // 计算位置
    QDesktopWidget *deskTop = QApplication::desktop();
    int curMonitor = deskTop->screenNumber(this);
    QRect deskRect = deskTop->screenGeometry(curMonitor);
    //
    auto showFun = [this, prompt, deskRect]() {
        prompt->adjustSize();
        prompt->setVisible(true);
        prompt->startToshow(QPoint(deskRect.right(),
                                   deskRect.top() + 80 + _arMessagePrompt.size() * 80));
        _arMessagePrompt.push_back(prompt);
    };

    // 设置最多显示 超过之后干掉 其他移动到上面
    if (_arMessagePrompt.size() >= 3) {
        for (MessagePrompt *tmpPrompt : _arMessagePrompt) {
            tmpPrompt->moveTop();
        }

        ;

        auto *tmpPrompt = _arMessagePrompt[0];

        //prompt->setParent(nullptr);
        delete tmpPrompt;

        tmpPrompt = nullptr;

        _arMessagePrompt.remove(0);
    }

    showFun();
#endif
}

#ifndef Q_OS_MAC
void ChatViewMainPanel::removePrompt(MessagePrompt *prompt)
{
    int pos = _arMessagePrompt.lastIndexOf(prompt);

    if (pos != -1) {
        {
            QMutexLocker locker(&_mutex);
            _arMessagePrompt.remove(pos);
        }
        prompt->setParent(nullptr);
        delete prompt;
        prompt = nullptr;
    }
}
#endif

void ChatViewMainPanel::updateUserConfig(const
                                         std::vector<st::entity::ImConfig> &configs)
{
    std::map<string, string> tmpNotice;
    auto it = configs.begin();

    for (; it != configs.end(); it++) {
        if (it->ConfigKey == "kNoticeStickJidDic") {
            tmpNotice[it->ConfigSubKey] = it->ConfigValue;
        }
    }

    //
    updateUserMaskName();
    //
    QMutexLocker locker(&_mutex);
    _mapNotice = tmpNotice;
}

void ChatViewMainPanel::updateUserConfig(const
                                         std::map<string, string> &deleteData,
                                         const std::vector<st::entity::ImConfig> &arImConfig)
{
    QMutexLocker locker(&_mutex);

    for (const auto &it : deleteData) {
        if (it.second == "kMarkupNames") {
            UID uid(it.first);

            if (!_mapItems.contains(uid)) {
                return;
            }

            auto *pViewItem = _mapItems.get(uid);

            if (pViewItem) {
                auto info = DB_PLAT.getUserInfo(uid.usrId(), true);
                string userName = st::getUserName(info);
                emit pViewItem->_pStatusWgt->updateName(userName.data());

                if (pViewItem->_pSearchMainWgt) {
                    emit pViewItem->_pSearchMainWgt->sgUpdateName(userName.data());
                }
            }
        } else if (it.second == "kNoticeStickJidDic") {
            _mapNotice.erase(it.first);
        }
    }

    //
    for (const auto &conf : arImConfig) {
        if (conf.ConfigKey == "kMarkupNames") {
            UID uid(conf.ConfigSubKey);

            if (!_mapItems.contains(uid)) {
                return;
            }

            auto *pViewItem = _mapItems.get(uid);

            if (pViewItem) {
                auto info = DB_PLAT.getUserInfo(uid.usrId(), true);
                string userName = st::getUserName(info);
                emit pViewItem->_pStatusWgt->updateName(userName.data());

                if (pViewItem->_pSearchMainWgt) {
                    emit pViewItem->_pSearchMainWgt->sgUpdateName(userName.data());
                }
            }
        } else if (conf.ConfigKey == "kNoticeStickJidDic") {
            _mapNotice[conf.ConfigSubKey] = conf.ConfigValue;
        }
    }
}

/**
 * 更新用户maskname
 */
void ChatViewMainPanel::updateUserMaskName()
{
    for (const auto &pItem : _mapItems.values()) {
        if (pItem && pItem->_chatType == st::Enum::TwoPersonChat) {
            auto info = DB_PLAT.getUserInfo(pItem->_uid.realId(), true);
            string userName = st::getUserName(info);

            if (pItem->_pStatusWgt) {
                emit pItem->_pStatusWgt->updateName(userName.data());
            }

            if (pItem->_pSearchMainWgt) {
                emit pItem->_pSearchMainWgt->sgUpdateName(userName.data());
            }
        }
    }
}

void ChatViewMainPanel::setFileMsgLocalPath(const string &key,
                                            const string &val)
{
    _pFileMsgConfig->setString(key, val);
    _pFileMsgConfig->saveConfig();
}

string ChatViewMainPanel::getFileMsgLocalPath(const string &key)
{
    string ret = _pFileMsgConfig->getString(key);
    return _pFileMsgConfig->getString(key);
}

void ChatViewMainPanel::onAppDeactivated()
{
    auto item = qobject_cast<ChatViewItem *>(_pStackedLayout->currentWidget());

    if (item && item->_pInputWgt) {
        item->_pInputWgt->onAppDeactivated();
        //        item->_pSearchMainWgt->setVisible(false);
    }
}

void ChatViewMainPanel::onShowChatWnd(QUInt8 chatType, QString userId,
                                      QString realJid,
                                      QString strUserName)
{
    //
    StSessionInfo stSession(chatType, std::move(userId), std::move(strUserName));
    stSession.realJid = std::move(realJid);
    emit sgSwitchCurFun(0);
    emit sgOpenNewSession(stSession);
    //
#ifndef _MACOS
    emit sgWakeUpWindow();
#endif
}

/**
 *
 * @param fromId
 * @param messageId
 */
void ChatViewMainPanel::recvBlackMessage(const string &fromId,
                                         const string &messageId)
{
    UID uid(fromId);

    if (!_mapItems.contains(uid)) {
        return;
    }

    auto *pViewItem = _mapItems.get(uid);

    if (pViewItem && _pViewItem->_pChatMainWgt) {
        _pViewItem->_pChatMainWgt->recvBlackListMessage(QString::fromStdString(
                                                            messageId));
    }
}

/**
 *
 * @param key
 * @param path
 */
void ChatViewMainPanel::addConllection(const QString &subkey,
                                       const QString &path)
{
    ChatMsgManager::setUserSetting(false, "kCollectionCacheKey",
                                   subkey.toStdString(), path.toStdString());
}

void ChatViewMainPanel::showEmptyLabel()
{
    if (nullptr == _pEmptyLabel) {
        _pEmptyLabel = new QLabel(this);
        _pEmptyLabel->setObjectName("ChatViewEmptyLabel");
        _pEmptyLabel->setAlignment(Qt::AlignCenter);
        _pStackedLayout->addWidget(_pEmptyLabel);
    }

    QTime curTime = QTime::currentTime();
    int hour = curTime.hour();
    QPixmap emptyPixmap;

    if (hour < 12)
        emptyPixmap = st::qimage::loadImage(
                          QString(":/chatview/image1/morning_%1.png").arg(
                              AppSetting::instance().getThemeMode()), false);
    else if (hour < 19)
        emptyPixmap = st::qimage::loadImage(
                          QString(":/chatview/image1/afternoon_%1.png").arg(
                              AppSetting::instance().getThemeMode()), false);
    else
        emptyPixmap = st::qimage::loadImage(
                          QString(":/chatview/image1/night_%1.png").arg(
                              AppSetting::instance().getThemeMode()), false);

    _pEmptyLabel->setPixmap(emptyPixmap);
    _pStackedLayout->setCurrentWidget(_pEmptyLabel);
}

/**
 * 更新群资料信息
 * @param groupinfo
 */
void ChatViewMainPanel::updateGroupInfo(const
                                        std::shared_ptr<st::StGroupInfo> &groupinfo)
{
    UID uid(groupinfo->groupId);

    if (!_mapItems.contains(uid)) {
        return;
    }

    auto *pViewItem = _mapItems.get(uid);

    if (pViewItem) {
        emit pViewItem->_pGroupSidebar->updateGroupTopic(QString::fromStdString(
                                                             groupinfo->title));
        emit pViewItem->_pStatusWgt->updateName(QString::fromStdString(
                                                    groupinfo->name));

        if (pViewItem->_pSearchMainWgt) {
            emit pViewItem->_pSearchMainWgt->sgUpdateName(QString::fromStdString(
                                                              groupinfo->name));
        }
    }
}

void deleteDictionary(const QString &dirPath)
{
    QFileInfo info(dirPath);

    if (info.isDir()) {
        QDir baseDir(dirPath);
        auto infoList = baseDir.entryInfoList(QDir::Dirs | QDir::Files |
                                              QDir::NoDotAndDotDot);

        for (const auto &tmpInfo : infoList) {
            if (tmpInfo.isDir()) {
                deleteDictionary(tmpInfo.absoluteFilePath());
                baseDir.rmpath(tmpInfo.absoluteFilePath());
            } else {
                QFile::remove(tmpInfo.absoluteFilePath());
            }
        }
    } else {
        QFile::remove(dirPath);
    }
}

void deleteHistoryLogs()
{
    QString logBasePath = QString("%1/logs").arg(
                              DC.getAppdataRoamingPath().data());
    QDir baseDir(logBasePath);
    auto infoList = baseDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const auto &info : infoList) {
        QString dirName = info.baseName();
        auto dirDate = QDate::fromString(dirName, "yyyy-MM-dd");
        auto now = QDate::currentDate();

        if (dirDate.addDays(5) <= now) {
            deleteDictionary(info.absoluteFilePath());
            baseDir.rmpath(info.absoluteFilePath());
        }
    }
}

void deleteDumps()
{
    QString logBasePath = QString("%1/logs").arg(
                              DC.getAppdataRoamingPath().data());
    //
#ifdef _WINDOWS
    std::function<void(const QString &)> delDmpfun;
    delDmpfun = [&delDmpfun](const QString & path) {
        QDir dir(path);
        QFileInfoList infoList = dir.entryInfoList(QDir::Dirs | QDir::Files |
                                                   QDir::NoDotAndDotDot);

        for (QFileInfo tmpInfo : infoList) {
            if (tmpInfo.isDir()) {
                delDmpfun(tmpInfo.absoluteFilePath());
            } else if (tmpInfo.isFile()) {
                if (tmpInfo.suffix().toLower() == "dmp") {
                    QFile::remove(tmpInfo.absoluteFilePath());
                }
            }
        }
    };
    delDmpfun(logBasePath);
#endif
}

/**
 * 打包发送日志 dump
 */
void ChatViewMainPanel::packAndSendLog(const QString &describe)
{
    QT_CONCURRENT_FUNC([this, describe]() {
        //
        int isNotify = ("@@#@@" != describe.toStdString());

        if (isNotify) {
            emit sgShowLiteMessageBox(true, tr("开始收集日志..."));
        }

        //db 文件
        auto appDataPath = QString::fromStdString(DC.getAppdataRoamingPath());
        auto userPath = QString::fromStdString(DC.getAppdataRoamingUserPath());
        QString logBasePath = appDataPath + "/logs";
        auto copyFileFun = [appDataPath, userPath, logBasePath](const QString & src,
        const QString & dest) {
            QString dbPath = userPath + src;
            QString logDbPath = logBasePath + dest;

            if (QFile::exists(logDbPath)) {
                QFile::remove(logDbPath);
            }

            if (QFile::exists(dbPath)) {
                QFile::copy(dbPath, logDbPath);
            }
        };
        copyFileFun("/qtalk.db", "/qtalk.db");
        copyFileFun("/qtalk.db-shm", "/qtalk.db-shm");
        copyFileFun("/qtalk.db-wal", "/qtalk.db-wal");
        copyFileFun("/../sysconfig", "/sysconfig");

        //
        if (isNotify) {
            emit sgShowLiteMessageBox(true, tr("日志收集完毕 开始打包..."));
        }

        QString logZip = logBasePath + "/log.zip";
        auto index = 0;
        bool packOk = false;

        while (index < 2 && !packOk) {
            // zip
            if (QFile::exists(logZip)) {
                QFile::remove(logZip);
            }

            bool ret = JlCompress::compressDir(logZip, logBasePath);

            if (ret) {
                QFileInfo logFileInfo(logZip);

                // 检测日志大小
                if (logFileInfo.size() >= 1024 * 1024 * 1024) {
                    //                if (isNotify)
                    //                    emit sgShowLiteMessageBox(true, "日志文件太大, 增加删除历史日志!");
                    if (index == 0) {
                        deleteHistoryLogs();
                    } else if (index == 1) {
                        deleteDumps();
                    }

                    index++;
                } else {
                    packOk = true;
                }
            } else {
                break;
            }
        }

        if (packOk) {
            if (isNotify) {
                emit sgShowLiteMessageBox(true,
                                          tr("日志打包成功 开始上传日志并发送邮件..."));
            }

            //
            ChatMsgManager::sendLogReport(describe.toStdString(), logZip.toStdString());
        } else {
            if (isNotify) {
                emit sgShowInfoMessageBox(tr("日志反馈失败，请重试..."));
            }
        }
    });
}

/**
* 截屏
*/
void ChatViewMainPanel::systemShortCut()
{
    _pSnipScreenTool->setConversionId(SYSTEM_CONVERSIONID);
    _pSnipScreenTool->Init().Start();
#ifdef _MACOS
    //onSnapFinish(SYSTEM_CONVERSIONID);
#endif
}

void ChatViewMainPanel::onSnapFinish(const QString &id)
{
    if (id == SYSTEM_CONVERSIONID) {
        //
        auto *item = qobject_cast<ChatViewItem *>(_pStackedLayout->currentWidget());

        if (nullptr != item) {
            _pSnipScreenTool->setConversionId(item->conversionId());
        }

        //
        QString localPath = QString("%1/image/temp/").arg(
                                DC.getAppdataRoamingUserPath().c_str());
        QString fileName = localPath +
                           QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz.png");
        bool bret = QApplication::clipboard()->pixmap().save(fileName, "PNG");

        if (bret) {
            // 将截图放到 剪切板
            auto *mimeData = new QMimeData;
            //            mimeData->setUrls(QList<QUrl>() << QUrl::fromLocalFile(fileName));
            mimeData->setImageData(QImage(fileName));
            QApplication::clipboard()->setMimeData(mimeData);

            if (_pStackedLayout->currentIndex() != -1) {
                if (this->isActiveWindow()) {
                    item = qobject_cast<ChatViewItem *>(_pStackedLayout->currentWidget());

                    if (nullptr != item) {
                        item->_pInputWgt->onPaste();
                    }
                }
            }
        }
    } else {
        if (_pViewItem) {
            _pViewItem->_pInputWgt->onSnapFinish(id);
        }
    }
}

void ChatViewMainPanel::onSnapCancel(const QString &id)
{
    if (id == SYSTEM_CONVERSIONID) {
    } else {
        if (_pViewItem) {
            _pViewItem->_pInputWgt->onSnapCancel(id);
        }
    }
}

/**
 * 清除会话
 * @param id
 */
void ChatViewMainPanel::onRemoveSession(const QString &id)
{
    UID uid(id);

    if (_pViewItem && _pViewItem->_uid == uid) {
        _pViewItem = nullptr;
        showEmptyLabel();
    }

    _mapItems.remove(uid);
}

/**
 *
 */
void ChatViewMainPanel::onRemoveGroupMember(const string &groupId,
                                            const string &memberId)
{
    //
    UID uid(groupId);

    if (!_mapItems.contains(uid)) {
        return;
    }

    auto *pViewItem = _mapItems.get(uid);

    if (pViewItem) {
        emit pViewItem->sgRemoveGroupMember(memberId);
    }
}

/**
 *
 */
void ChatViewMainPanel::reloadChatView()
{
    QMutexLocker locker(&_mutex);
    //
    showEmptyLabel();
    //
    auto keys = _mapItems.keys();

    for (const auto &id : keys) {
        _mapItems.remove(id);
    }

    _pViewItem = nullptr;
}

//
void ChatViewMainPanel::showLiteMessageBox(bool isSuccess,
                                           const QString &message)
{
    if (isSuccess) {
        LiteMessageBox::success(message, 2000);
    } else {
        LiteMessageBox::failed(message, 2000);
    }
}

void ChatViewMainPanel::showInfoMessageBox(const QString &message)
{
    QtMessageBox::information(this, tr("提示"), message);
}

//
void ChatViewMainPanel::onLogReportMessageRet(bool isSuccess,
                                              const string &message)
{
    emit sgShowInfoMessageBox(QString::fromStdString(message));
}

/**
 *
 */
void ChatViewMainPanel::recvUserStatus(const string &user,
                                       const string &userstatus)
{
    //
    UID uid(user);
    auto *pViewItem = _mapItems.get(uid);

    if (pViewItem) {
        emit pViewItem->sgUpdateUserStatus(userstatus.data());
    }
}

/**
 * 播放收到消息的提示音
 */
void ChatViewMainPanel::playSound()
{
#ifndef _MACOS

    if (_pPlayer->state() != QMediaPlayer::StoppedState) {
        _pPlayer->stop();
    }

    _pPlayer->play();
#else
    QString dirctionPath = QString::fromStdString(
                               AppSetting::instance().getgetNewMsgAudioPath());
    const QString &soundPath = ":/chatview/sound/msg.wav";

    if ((dirctionPath.isEmpty() || !QFile::exists(dirctionPath))
        && QFile::exists(soundPath)) {
        dirctionPath = QString("%1/msg.wav").arg(DC.getAppdataRoamingPath().data());
        QTemporaryFile *tmpFile = QTemporaryFile::createNativeFile(
                                      soundPath); // Returns a pointer to a temporary file
        tmpFile->copy(dirctionPath);
        AppSetting::instance().setgetNewMsgAudioPath(dirctionPath.toStdString());
    }

    if (QFile::exists(dirctionPath)) {
        st::mac::util::PlayAudioSound::PlaySound(
            dirctionPath.toStdString().c_str());
    }

#endif
}

void ChatViewMainPanel::initSound()
{
#ifndef _MACOS
    _pPlayer = new QMediaPlayer;
    const QString &soundPath = ":/chatview/sound/msg.wav";
    QString dirctionPath = QString::fromStdString(
                               AppSetting::instance().getgetNewMsgAudioPath());

    if ((dirctionPath.isEmpty() || !QFile::exists(dirctionPath))
        && QFile::exists(soundPath)) {
        dirctionPath = QString("%1/msg.wav").arg(DC.getAppdataRoamingPath().data());
        QTemporaryFile *tmpFile = QTemporaryFile::createNativeFile(
                                      soundPath); // Returns a pointer to a temporary file
        tmpFile->copy(dirctionPath);
        AppSetting::instance().setgetNewMsgAudioPath(dirctionPath.toStdString());
    }

    if (QFile::exists(dirctionPath)) {
        _pPlayer->setMedia(QUrl::fromLocalFile(dirctionPath));
    }

#endif
#ifndef _WINDOWS
    _pVoicePlayer = new QMediaPlayer;
#endif // !_WINDOWS
}

#ifdef _WINDOWS
void play(const QString &filePath, VoiceMessageItem *msgItem)
{
    QFile amrfile(filePath);

    if (!amrfile.open(QIODevice::ReadOnly)) {
        return;
    }

    QByteArray m_buffer = amrfile.readAll();
    QByteArray *spArrData = new QByteArray;

    if (VoiceHelper::amrBufferToWavBuffer((unsigned char *)m_buffer.data(),
                                          m_buffer.size(), spArrData)) {
        if (spArrData->size() == 0) {
            return;
        }

        QAudioFormat format;
        format.setSampleRate(8000);
        format.setChannelCount(1);
        format.setSampleSize(16);
        format.setCodec("audio/pcm");
        format.setByteOrder(QAudioFormat::LittleEndian);
        format.setSampleType(QAudioFormat::UnSignedInt);

        if (!spOutputDevice.isNull()) {
            spOutputDevice->disconnect();
        }

        spAudioOutput.reset(new QAudioOutput(format));
        spOutputDevice.reset(new WavOutput(spArrData));
        WavOutput *od = spOutputDevice.data();
        QObject::connect(od, &WavOutput::sgFinish,
        g_pMainPanel, [msgItem, spArrData]() {
            spAudioOutput->stop();
            delete spArrData;
            msgItem->stopVoice();
        });
        spAudioOutput->start(spOutputDevice.data());
    }
}
#endif

void ChatViewMainPanel::playVoice(const string &localFile,
                                  VoiceMessageItem *msgItem)
{
#ifndef _WINDOWS

    if (_pVoicePlayer) {
        if (_pVoicePlayer->state() == QMediaPlayer::PlayingState) {
            _pVoicePlayer->stop();
        }

        _pVoicePlayer->setMedia(QUrl::fromLocalFile(localFile.data()));
        _pVoicePlayer->play();
        connect(_pVoicePlayer, &QMediaPlayer::stateChanged, this,  [msgItem](
        QMediaPlayer::State newState) {
            if (newState == QMediaPlayer::StoppedState) {
                msgItem->stopVoice();
            }
        });
    }

#else
    play(localFile.data(), msgItem);
#endif // !_WINDOWS
}

///**
// * 2人视频
// */
//void ChatViewMainPanel::onRecvVideo(const string &userId) {
//
//}

void ChatViewMainPanel::onRecvAddGroupMember(const string &groupId,
                                             const string &memberId,
                                             const string &nick, int affiliation)
{
    UID uid(groupId);
    QMutexLocker locker(&_mutex);
    {
        if (!_mapItems.contains(uid)) {
            return;
        }

        auto *pViewItem = _mapItems.get(uid);

        if (pViewItem) {
            pViewItem->onRecvAddGroupMember(memberId, nick, affiliation);
        }
    }
}

void ChatViewMainPanel::onRecvRemoveGroupMember(const string &groupId,
                                                const string &memberId)
{
    UID uid(groupId);

    if (!_mapItems.contains(uid)) {
        return;
    }

    auto *pViewItem = _mapItems.get(uid);

    if (pViewItem) {
        pViewItem->onRecvRemoveGroupMember(memberId);
    }
}

void ChatViewMainPanel::onShowVideoWnd(const string &caller,
                                       const string &callee, bool isVideo)
{
    start2Talk_old(caller, isVideo, false);
}

void ChatViewMainPanel::showShowCodeWnd(const QString &type,
                                        const QString &language, const QString &content)
{
    if (nullptr == _pCodeShowWnd) {
        _pCodeShowWnd = new CodeShowWnd(this);
    }

    if (_pCodeShowWnd) {
        _pCodeShowWnd->show();
        _pCodeShowWnd->hide();
        _pCodeShowWnd->showCode(type, language, content);
        _pCodeShowWnd->showCenter(false, this);
        QApplication::setActiveWindow(_pCodeShowWnd);
    }
}

void ChatViewMainPanel::showSendCodeWnd(const UID &uid)
{
    if (_pSendCodeWnd == nullptr) {
        _pSendCodeWnd = new SendCodeWnd(this);
    }

    if (_pSendCodeWnd && _pViewItem) {
        _pSendCodeWnd->show();
        _pSendCodeWnd->hide();
        QString code = _pViewItem->_pInputWgt->toPlainText();
        _pSendCodeWnd->addCode(uid, code);

        //
        if (_pSendCodeWnd->isVisible()) {
            _pSendCodeWnd->setVisible(false);
        }

        _pSendCodeWnd->showCenter(false, nullptr);
    }
}

/**
 * send code message
 * @param uid
 * @param text
 * @param codeType
 * @param codeLanguage
 */
void ChatViewMainPanel::sendCodeMessage(const UID &uid, const string &text,
                                        const string &codeType,
                                        const string &codeLanguage)
{
    if (!_mapItems.contains(uid)) {
        return;
    }

    auto *pViewItem = _mapItems.get(uid);

    if (pViewItem) {
        sendCodeMessage(uid, _pViewItem->_chatType, text, codeType, codeLanguage);
    }
}

/**
 *
 * @param localHead
 */
void ChatViewMainPanel::onChangeHeadSuccess(const string &localHead)
{
    string xmppId = getSelfUserId();
    {
        QMutexLocker locker(&_mutex);
        _mapHeadPath[QString::fromStdString(xmppId)] = QString::fromStdString(
                                                           localHead);
    }
}

/**
 *
 * @param mood
 */
void ChatViewMainPanel::onUpdateMoodSuccess(const string &userId,
                                            const string &mood)
{
    UID uid(userId);

    if (!_mapItems.contains(uid)) {
        return;
    }

    auto *pViewItem = _mapItems.get(uid);

    if (pViewItem && pViewItem->_pStatusWgt) {
        emit pViewItem->_pStatusWgt->updateMood(QString::fromStdString(mood));
    }
}

/**
 *
 */
void ChatViewMainPanel::onAppActive()
{
}

/**
 *
 */
void ChatViewMainPanel::onMsgSoundChanged()
{
#ifdef _WINDOW
    _pPlayer->setMedia(QUrl());
#endif
    QString soundPath = QFileDialog::getOpenFileName(this,
                                                     tr("请选择一个提示音"), DC.getHistoryDir().data(),
                                                     "mp3 wav(*.mp3 *.wav)");

    if (soundPath.isEmpty()) {
        return;
    }

    const QString &dirctionPath = QString::fromStdString(
                                      AppSetting::instance().getgetNewMsgAudioPath());
    QFile::remove(dirctionPath);
    QString newSoundPath = QString("%1/%2").arg(
                               DC.getAppdataRoamingPath().data()).arg(QFileInfo(soundPath).fileName());

    if (QFileInfo(newSoundPath).exists() && !QFile::remove(dirctionPath)) {
        emit sgShowLiteMessageBox(false, tr("提示音 文件替换失败"));
        return;
    }

    QFile::copy(soundPath, newSoundPath);
    AppSetting::instance().setgetNewMsgAudioPath(newSoundPath.toStdString());
#ifndef _MACOS

    if (QFile::exists(newSoundPath)) {
        _pPlayer->setMedia(QUrl::fromLocalFile(newSoundPath));
    }

#else
    st::mac::util::PlayAudioSound::removeSound(
        dirctionPath.toStdString().data());
#endif
}

/**
 *
 * @param localFile
 */
string ChatViewMainPanel::uploadFile(const string &localFile)
{
    string ret;
    ret = ChatMsgManager::uploadFile(localFile);
    return ret;
}

/**
 *
 * @param url
 */
void ChatViewMainPanel::sendShareMessage(const QString &id, int chatType,
                                         const QString &shareUrl)
{
    UID uid(id);
    //
    _pSelectUserWnd->reset();
    _pSelectUserWnd->showCenter(true, this);
    _pSelectUserWnd->_loop->quit();
    _pSelectUserWnd->_loop->exec();

    //
    if (_pSelectUserWnd->getEvtRet() != 1
        || _pSelectUserWnd->_selectedIds.empty()) {
        qInfo() << "cancel send share message";
        return;
    }

    //
    QString title;

    if (chatType == st::Enum::GroupChat) {
        auto info = DB_PLAT.getGroupInfo(uid.usrId());

        if (info) {
            title = QString("%1的聊天记录").arg(info->Name.data());
        } else {
            title = QString(tr("群聊记录"));
        }
    } else {
        string userId = uid.usrId();
        string selfName = DC.getSelfName();

        if (userId == getSelfUserId()) {
            title = QString("%1的聊天记录").arg(selfName.data());
        } else {
            auto info = DB_PLAT.getUserInfo(uid.usrId());

            if (info) {
                title = QString("%1和%2的聊天记录").arg(info->Name.data()).arg(
                            selfName.data());
            } else {
                title = QString("%1的聊天记录").arg(selfName.data());
            }
        }
    }

    QString linkurl = QString("%1?jdata=%2").arg(
                          NavigationManager::instance().getShareUrl().data()).arg(QUrl(
                                      shareUrl.toUtf8().toBase64()).toEncoded().data());
    nJson obj;
    obj["title"] = title.toUtf8();
    obj["desc"] = "";
    obj["linkurl"] = linkurl.toUtf8();
    string extenInfo = obj.dump();
    // 发送消息
    long long send_time =
        QDateTime::currentDateTime().toMSecsSinceEpoch() - DC.getServerDiffTime() *
        1000;
    st::entity::ImMessageInfo message;
    message.From = getSelfUserId();
    message.LastUpdateTime = send_time;
    message.Type = st::entity::MessageTypeCommonTrdInfo;
    message.Content =
        QObject::tr("收到了一个消息记录文件文件，请升级客户端查看。").toStdString();
    message.ExtendedInfo = extenInfo;
    message.Direction = st::entity::MessageDirectionSent;
    message.UserName = DC.getSelfName();
    S_Message e;
    e.time = send_time;

    //
    if (_pSelectUserWnd->getEvtRet() == 1
        && !_pSelectUserWnd->_selectedIds.empty()) {
        auto it = _pSelectUserWnd->_selectedIds.begin();

        for (; it != _pSelectUserWnd->_selectedIds.end(); it++) {
            UID toUid(it->first.usrId(), it->first.realId());
            message.ChatType = it->second;
            message.To = it->first.usrId();
            message.RealJid = it->first.realId();
            message.XmppId = it->first.usrId();
            message.MsgId = st::utils::uuid();
            e.message = message;
            e.userId = it->first.usrId();
            e.chatType = it->second;

            if (!_mapItems.contains(uid)) {
                return;
            }

            auto *pViewItem = _mapItems.get(toUid);

            if (pViewItem) {
                pViewItem->showMessage(message, ChatMainWgt::EM_JUM_BOTTOM);
            }

            emit sgUserSendMessage();
            ChatMsgManager::sendMessage(e);
        }
    }
}

/**
 *
 * @param type
 * @param key
 */
void ChatViewMainPanel::searchLocalSession(int type, const string &key)
{
    //
    std::vector<st::StShareSession> sessions;

    switch (type) {
    case SelectUserWnd::EM_TYPE_RECENT:
        ChatMsgManager::getRecentSession(sessions);
        break;

    case SelectUserWnd::EM_TYPE_CONTACTS:
        break;

    case SelectUserWnd::EM_TYPE_GROUPCHAT:
        break;

    default:
        break;
    }

    if (_pSelectUserWnd) {
        _pSelectUserWnd->setSessions(type, sessions);
    }
}

void ChatViewMainPanel::forwardMessage(const string &messageId)
{
    _pSelectUserWnd->reset();
    _pSelectUserWnd->showCenter(true, this);
    _pSelectUserWnd->_loop->quit();
    _pSelectUserWnd->_loop->exec();

    if (_pSelectUserWnd->getEvtRet() != 1
        || _pSelectUserWnd->_selectedIds.empty()) {
        qInfo() << "cancel forward message";
        return;
    }

    //
    std::map<string, int> users;
    auto it = _pSelectUserWnd->_selectedIds.begin();

    for (; it != _pSelectUserWnd->_selectedIds.end(); it++) {
        users[it->first.usrId()] = it->second;
    }

    //
    ChatMsgManager::forwardMesssage(messageId, users);
}

/**
 *
 * @param filePath
 * @param linkFile
 */
void ChatViewMainPanel::makeFileLink(const QString &filePath,
                                     const QString &fileMd5)
{
    QString linkDir = QString("%1/fileLink").arg(
                          DC.getAppdataRoamingUserPath().data());

    if (!QFile::exists(linkDir)) {
        QDir dir;
        dir.mkpath(linkDir);
    }

#ifdef _WINDOWS
    QString linkFile = linkDir.append("/").append(fileMd5).append(".lnk");
#else
    QString linkFile = linkDir.append("/").append(fileMd5);
#endif // _WINDOWS
    // 保证最新
    QFile::remove(linkFile);
    // 创建软链
    bool mkLink = QFile::link(filePath, linkFile);
    debug_log("make file link {0} -> {1} result:{2}", filePath.toStdString(),
              linkFile.toStdString(), mkLink);
}

QString ChatViewMainPanel::getFileLink(const QString &fileMd5)
{
    if (!fileMd5.isEmpty()) {
        return QString("%1/fileLink/%2").arg(
                   DC.getAppdataRoamingUserPath().data()).arg(fileMd5);
    }

    return QString();
}

/**
 *
 */
void ChatViewMainPanel::setAutoReplyFlag(bool flag)
{
    _autoReply = flag;
}

void ChatViewMainPanel::sendAutoPeplyMessage(const st::entity::UID &uid,
                                             const string &messageId)
{
    bool atuoReplyFalg = AppSetting::instance().getAutoReplyEnable();

    if (!atuoReplyFalg) {
        return;
    }

    int hour = QTime::currentTime().hour();
    bool awaysReply = AppSetting::instance().getAwaysAutoReply();
    bool within = AppSetting::instance().withinEnableTime(hour);
    auto flag = awaysReply ? within : (_autoReply && within);

    if (flag) {
        string autoReplyMessage = AppSetting::instance().getAutoReplyMsg();
        long long send_time =
            QDateTime::currentDateTime().toMSecsSinceEpoch() - DC.getServerDiffTime() *
            1000;
        string msgId = "auto_reply_" + messageId;
        st::entity::ImMessageInfo message;
        message.ChatType = st::Enum::TwoPersonChat;
        message.MsgId = msgId;
        message.To = uid.usrId();
        message.RealJid = uid.usrId();
        message.From = getSelfUserId();
        message.LastUpdateTime = send_time;
        message.XmppId = uid.usrId();
        message.Type = st::entity::MessageTypeText;
        message.Content = QString("[自动回复]: %1").arg(
                              autoReplyMessage.data()).toStdString();
        message.UserName = DC.getSelfName();
        message.AutoReply = true;
        message.Direction = st::entity::MessageDirectionSent;
        S_Message e;
        e.message = message;
        e.userId = uid.usrId();
        e.time = send_time;
        e.chatType = st::Enum::TwoPersonChat;
        ChatMsgManager::sendMessage(e);
        //        UID uid(receiver);
        //        if(_mapSession.contains(uid))
        //            _pViewItem->showMessage(message, false);
    }
}

void ChatViewMainPanel::setSearchStyle(const QString &qss)
{
    _qss = qss;
    //
    //  for(const auto& pItem : _mapItems.values())
    //    {
    ////        auto *pItem = it.second;
    //        if(pItem && pItem->_pSearchMainWgt)
    //            pItem->_pSearchMainWgt->initStyle(qss);
    //    }
    //    resize(this->width() - 10, this->height());
    //    resize(this->width() + 10, this->height());
}

void ChatViewMainPanel::onDisConnected()
{
    {
        QMutexLocker locker(&_sendingMutex);
        _sending.clear();
    }

    for (const auto &pViewItem : _mapItems.values()) {
        if (pViewItem && pViewItem->_pChatMainWgt) {
            emit pViewItem->_pChatMainWgt->sgDisConnected();
        }
    }
}

/**
 *
 * @param msgInfo
 */
void ChatViewMainPanel::resendMessage(MessageItemBase *item)
{
    QApplication::processEvents(QEventLoop::AllEvents, 100);
    UID uid;
    const auto &message = item->_msgInfo;

    if (message.type == st::Enum::TwoPersonChat) {
        uid = UID(message.real_id);
    } else if (message.type == st::Enum::GroupChat) {
        uid = UID(message.real_id);
    } else if (message.type == st::Enum::Consult) {
        uid = UID(message.from, message.real_id);
    } else if (message.type == st::Enum::ConsultServer) {
        uid = UID(message.xmpp_id, message.real_id);
    }

    {
        switch (message.msg_type) {
        case st::entity::MessageTypeText:
        case st::entity::MessageTypeGroupAt: {
            QT_CONCURRENT_FUNC([message, uid, item, this]() {
                std::map<string, string> mapAt;
                bool success = false;
                QString content = message.body;

                if (content.isEmpty()) {
                    content = getRealText(message.extend_info, message.msg_id, success, mapAt);
                } else {
                    success = true;
                }

                if (success) {
                    sendTextMessage(uid, message.type, content.toStdString(), mapAt,
                                    message.msg_id.toStdString());
                } else {
                    if (item) {
                        item->onDisconnected();
                    }

                    error_log("{0} message resend failed reason: convert message item failed",
                              message.msg_id.toStdString());
                    return;
                }
            });
            break;
        }

        case st::entity::MessageTypeSourceCode: {
            auto *codeItem = qobject_cast<CodeItem *>(item);

            if (codeItem) {
                sendCodeMessage(uid, message.type, codeItem->_code.toStdString(),
                                codeItem->_codeStyle.toStdString(), codeItem->_codeLanguage.toStdString(),
                                message.msg_id.toStdString());
            } else {
                item->onDisconnected();
                error_log("{0} message resend failed reason: convert message item failed",
                          message.msg_id.toStdString());
                QtMessageBox::warning(this, QObject::tr("警告"), tr("消息重发失败!"));
            }

            break;
        }

        case st::entity::MessageTypeFile: {
            nJson content = Json::parse(message.extend_info.toUtf8().data());

            if (nullptr == content) {
                item->onDisconnected();
                error_log("{0} message resend failed reason: file not exists",
                          message.msg_id.toStdString());
                QtMessageBox::warning(this, QObject::tr("警告"),
                                      tr("原文件不存在, 消息重发失败!"));
                break;
            }

            //
            string filePath = Json::get<string>(content, "FilePath");
            QFileInfo info(filePath.data());

            if (info.exists() && info.isFile()) {
                sendFileMessage(uid, message.type, filePath.data(), message.file_info.fileName,
                                message.file_info.fileSize, message.msg_id.toStdString());
            } else {
                if (item) {
                    item->onDisconnected();
                }

                error_log("{0} message resend failed reason: file not exists",
                          message.msg_id.toStdString());
                QtMessageBox::warning(this, QObject::tr("警告"),
                                      tr("原文件不存在, 消息重发失败!"));
            }

            break;
        }

        case st::entity::MessageTypeImageNew:
        case st::entity::MessageTypePhoto: {
            if (_pViewItem) {
                QString id = _pViewItem->conversionId();
                sendEmoticonMessage(id, message.body, false, message.msg_id.toStdString());
            }

            break;
        }

        default:
            break;
        }
    }
}

void dealImage(QString &strRet, QString imagePath, const QString &imageLink)
{
    QString srcImgPath = st::GetSrcImagePathByUrl(
                             imageLink.toStdString()).data();
    QFileInfo srcInfo(srcImgPath);

    if (srcInfo.exists() && srcInfo.isFile()) {
        imagePath = srcImgPath;
    }

    // load image
    string realSuffix = st::qimage::getRealImageSuffix(
                            imagePath).toStdString();
    QImage image;
    image.load(imagePath, realSuffix.data());
    int w = image.width();
    int h = image.height();
    // 移动原图处理
    auto tempPath = DC.getAppdataRoamingUserPath() + "/image/temp";

    if (!imageLink.isEmpty() && !imagePath.isEmpty() && !srcInfo.exists()
        && !imagePath.startsWith(tempPath.data())) {
        QFile::copy(imagePath, srcImgPath);
    }

    strRet.append(
        QString("[obj type=\"image\" value=\"%3\" width=%2 height=%1]").arg(h).arg(
            w).arg(imageLink));
}

QString ChatViewMainPanel::getRealText(const QString &json,
                                       const QString &msgId, bool &success, std::map<string, string> &mapAt)
{
    success = false;
    QString strRet;
    //
    auto document = QJsonDocument::fromJson(json.toUtf8());

    if (document.isNull()) {
    } else {
        QJsonArray array = document.array();

        for (auto &&i : array) {
            QJsonObject obj = i.toObject();
            int key = obj.value("key").toInt();
            QString value = obj.value("value").toString();

            switch (key) {
            case Type_Text: {
                strRet.append(value);
                break;
            }

            case Type_Image: {
                int index = 3;
                //
                QFileInfo info(value);

                if (!info.exists() || info.isDir()) {
                    index = -1;
                }

                //
                QString imageLink = obj.value("info").toString();

                if (!imageLink.isEmpty()) {
                    dealImage(strRet, value, imageLink);
                    break;
                }

                // 翻转异常图片
                {
                    QImageReader reader(value);

                    if (reader.transformation() != QImageIOHandler::TransformationNone) {
                        reader.setAutoTransform(true);
                        auto image = reader.read();
                        QString localPath = QString("%1/image/temp/").arg(
                                                DC.getAppdataRoamingUserPath().c_str());
                        QString fileName = localPath +
                                           QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz." + info.suffix());
                        image.save(fileName);
                        value = fileName;
                    }
                }

                //
                while (index > 0) {
                    imageLink = ChatMsgManager::getNetFilePath(
                                    string((const char *)value.toLocal8Bit()))
                                .data();

                    if (!imageLink.isEmpty()) {
                        dealImage(strRet, value, imageLink);
                        break;
                    }

                    index--;
                }

                //
                if (index <= 0) {
                    success = false;
                    //                        emit _pMainWgt->sgSendFailed(msgId);
                    emit sgShowInfoMessageBox(tr("消息发送失败"));
                    return QString();
                }

                break;
            }

            case Type_At: {
                QString xmppId = obj.value("info").toString();
                mapAt[xmppId.toStdString()] = value.toStdString();
                strRet.append(QString("@%1 ").arg(value));
                break;
            }

            case Type_Url: {
                strRet.append(QString("[obj type=\"url\" value=\"%1\"]").arg(value));
                break;
            }

            case Type_Invalid:
            default:
                break;
            }
        }
    }

    //先替换全角的@
    strRet.replace("＠", "@");
    //    替换非法字符
    QRegExp regExp("[\\x00-\\x08\\x0b-\\x0c\\x0e-\\x1f]");
    strRet.replace(regExp, "");

    //     去掉最后一个换行
    while (strRet.right(1) == "\n") {
        strRet = strRet.mid(0, strRet.size() - 1);
    }

    success = true;
    return strRet;
}

void ChatViewMainPanel::scanQRcCodeImage(const QString &imgPath)
{
    QPixmap pix = st::qimage::loadImage(imgPath, false);
    ScanQRcode::qzxingDecodeImage(pix);
}

void ChatViewMainPanel::preSendMessage(const st::entity::UID &uid,
                                       int chatType,
                                       int msgType, const QString &info, const string &messageId)
{
    if (g_pMainPanel) {
        long long send_time =
            QDateTime::currentDateTime().toMSecsSinceEpoch() - DC.getServerDiffTime() *
            1000;
        st::entity::ImMessageInfo message;
        message.ChatType = chatType;
        message.MsgId = messageId;
        message.To = uid.usrId();
        message.RealJid = uid.realId();
        message.XmppId = uid.usrId();
        message.From = g_pMainPanel->getSelfUserId();
        message.LastUpdateTime = send_time;
        message.Type = msgType;
        message.Content = "";
        message.ExtendedInfo = info.toStdString();
        message.Direction = st::entity::MessageDirectionSent;
        message.UserName = DC.getSelfName();

        if (_pViewItem && _pViewItem->_uid == uid) {
            _pViewItem->showMessage(message, ChatMainWgt::EM_JUM_BOTTOM);
        }

        emit g_pMainPanel->sgUserSendMessage();
        // 消息处理
        ChatMsgManager::preSendMessage(message);

        //
        if (_his_input_data.contains(uid)) {
            _his_input_data.remove(uid);
        }
    }
}

/**
  * @函数名   sendTextMessage
  * @功能描述 发送文本消息
  * @参数
  * @author   cc
  * @date     2018/09/19
  */
void ChatViewMainPanel::sendTextMessage(const st::entity::UID &uid,
                                        int chatType,
                                        const string &text, const std::map<string, string> &mapAt,
                                        const string &messageId)
{
    //
    if (text.empty()) {
        emit g_pMainPanel->sgShowInfoMessageBox(tr("无效的空消息"));
        return;
    }

    if (g_pMainPanel) {
        long long send_time =
            QDateTime::currentDateTime().toMSecsSinceEpoch() - DC.getServerDiffTime() *
            1000;
        string msgId = messageId.empty() ? st::utils::uuid() :
                       messageId;
        st::entity::ImMessageInfo message;
        S_Message e;
        message.ChatType = chatType;
        message.To = uid.usrId();
        message.RealJid = uid.realId();
        message.XmppId = uid.usrId();
        e.userId = uid.usrId();
        message.MsgId = msgId;
        message.From = g_pMainPanel->getSelfUserId();
        message.LastUpdateTime = send_time;
        message.Type = mapAt.empty() ? st::entity::MessageTypeText :
                       st::entity::MessageTypeGroupAt;
        message.Content = text;
        message.Direction = st::entity::MessageDirectionSent;
        message.UserName = DC.getSelfName();
        // 发送消息
        string backupInfo;

        if (!mapAt.empty()) {
            nJson objs;
            nJson obj;
            obj["type"] = 10001;
            nJson datas;
            auto it = mapAt.begin();

            for (; it != mapAt.end(); it++) {
                nJson data;
                data["jid"] = it->first.c_str();
                data["text"] = it->second.c_str();
                datas.push_back(data);
            }

            obj["data"] = datas;
            objs.push_back(obj);
            backupInfo = objs.dump();
        }

        //
        message.BackupInfo = backupInfo;
        //
        e.message = message;
        e.chatType = message.ChatType;
        e.time = send_time;
        emit g_pMainPanel->sgUserSendMessage();
        ChatMsgManager::sendMessage(e);

        // 显示消息
        if (!_mapItems.contains(uid)) {
            return;
        }

        auto *pViewItem = _mapItems.get(uid);

        if (pViewItem) {
            pViewItem->showMessage(message, ChatMainWgt::EM_JUM_BOTTOM);
        }
    }
}

void ChatViewMainPanel::sendCodeMessage(const st::entity::UID &uid,
                                        int chatType,
                                        const string &text,
                                        const string &codeType,
                                        const string &codeLanguage,
                                        const string &messageId)
{
    if (g_pMainPanel) {
        nJson obj;
        obj["CodeDecode"] = "";
        obj["CodeType"] = codeLanguage.data();
        obj["CodeStyle"] = codeType.data();
        obj["Code"] = text.data();
        string extenInfo = obj.dump();
        // 发送消息
        long long send_time =
            QDateTime::currentDateTime().toMSecsSinceEpoch() - DC.getServerDiffTime() *
            1000;
        string msgId = messageId.empty() ? utils::uuid() : messageId;
        st::entity::ImMessageInfo message;
        message.ChatType = chatType;
        message.MsgId = msgId;
        message.To = uid.usrId();
        message.RealJid = uid.realId();
        message.XmppId = uid.usrId();
        message.From = g_pMainPanel->getSelfUserId();
        message.LastUpdateTime = send_time;
        message.Type = st::entity::MessageTypeSourceCode;
        message.Content = text;
        message.ExtendedInfo = extenInfo;
        message.Direction = st::entity::MessageDirectionSent;
        message.UserName = DC.getSelfName();
        S_Message e;
        e.message = message;
        e.userId = uid.usrId();
        e.time = send_time;
        e.chatType = chatType;
        emit g_pMainPanel->sgUserSendMessage();
        ChatMsgManager::sendMessage(e);

        if (!_mapItems.contains(uid)) {
            return;
        }

        auto *pViewItem = _mapItems.get(uid);

        if (pViewItem) {
            pViewItem->showMessage(message, ChatMainWgt::EM_JUM_BOTTOM);
        }
    }
}

/**
  * @函数名   sendFileMessage
  * @功能描述 发送文件消息
  * @参数
  * @author   cc
  * @date     2018/10/16
  * //为了处理windows字符集导致的显示问题 发送消息时FilePath 用local字符集 记录用utf8字符集
  */
void ChatViewMainPanel::sendFileMessage(const st::entity::UID &uid,
                                        int chatType,
                                        const QString &filePath,
                                        const QString &fileName,
                                        const QString &fileSize,
                                        const string &messageId)
{
    QString tmp_file_path(filePath);

    // 上传文件
    if (g_pMainPanel) {
        long long send_time =
            QDateTime::currentDateTime().toMSecsSinceEpoch() - DC.getServerDiffTime() *
            1000;
        string msgId = messageId.empty() ? utils::uuid() : messageId;
        // 本地显示
        st::entity::ImMessageInfo message;
        message.ChatType = chatType;
        message.MsgId = msgId;
        message.To = uid.usrId();
        message.RealJid = uid.realId();
        message.XmppId = uid.usrId();
        message.From = g_pMainPanel->getSelfUserId();
        message.LastUpdateTime = send_time;
        message.Type = st::entity::MessageTypeFile;
        message.Direction = st::entity::MessageDirectionSent;
        message.UserName = DC.getSelfName();
        message.FileName = fileName.toStdString();
        message.FileSize = fileSize.toStdString();
        QFuture<void> future = QT_CONCURRENT_FUNC([tmp_file_path, &message]() {
            message.FileMd5 = st::utils::getFileMd5(tmp_file_path.toLocal8Bit().data());
        });

        //        future.waitForFinished();
        while (!future.isFinished()) {
            QApplication::processEvents(QEventLoop::AllEvents, 100);
        }

#ifdef _WINDOWS

        if (message.FileMd5.empty()) {
            QFileInfo fileInfo(tmp_file_path);

            if (fileInfo.exists() && fileInfo.size() < 200 * 1024 * 1024) {
                tmp_file_path = QString("%1/%2.%3").arg(
                                    DC.getAppdataRoamingUserPath().data()).arg(
                                    QDateTime::currentMSecsSinceEpoch()).arg(fileInfo.completeSuffix());
                QFile::copy(filePath, tmp_file_path);
                message.FileMd5 = st::utils::getFileMd5(tmp_file_path.toLocal8Bit().data());
            } else {
                emit g_pMainPanel->sgShowInfoMessageBox(
                    tr("文件路径问题，导致文件上传失败，请确认!"));
                return;
            }
        }

#endif // _WINDOWS
        {
            nJson content;
            content["FileName"] = message.FileName;
            content["FileSize"] = message.FileSize;
            content["FILEMD5"] = message.FileMd5;
            content["FilePath"] = filePath.toStdString().data();
            message.ExtendedInfo = content.dump();
        }
        // 创建软链
        g_pMainPanel->makeFileLink(filePath, message.FileMd5.data());

        // 显示消息
        if (!_mapItems.contains(uid)) {
            return;
        }

        auto *pViewItem = _mapItems.get(uid);

        if (pViewItem && messageId.empty()) {
            pViewItem->showMessage(message, ChatMainWgt::EM_JUM_BOTTOM);
            ChatMsgManager::preSendMessage(message);
        }

        // 上传文件 发送消息
        QT_CONCURRENT_FUNC([this, uid, chatType, tmp_file_path, filePath, message,
              send_time, msgId]() {
            S_Message e;
            e.message = message;
            e.userId = uid.usrId();
            e.time = send_time;
            e.chatType = chatType;
            // get url
            string url = ChatMsgManager::uploadFile(tmp_file_path.toLocal8Bit().data(),
                                                    true, msgId);
            e.message.FileUrl = url;

            //
            if (filePath != tmp_file_path) {
                QFile::remove(tmp_file_path);
            }

            if (!url.empty()) {
                if (_pViewItem && _pViewItem->_uid == uid && _pViewItem->_pChatMainWgt) {
                    emit _pViewItem->_pChatMainWgt->sgUploadFileSuccess(msgId.data(), url.data());
                }

                nJson content;
                content["FILEID"] = message.MsgId;
                content["FileName"] = message.FileName;
                content["FileSize"] = message.FileSize;
                content["FILEMD5"] = message.FileMd5;
                content["HttpUrl"] = url;
                string strContent = content.dump();
                e.message.Content = strContent;
                e.message.ExtendedInfo = strContent;
                emit g_pMainPanel->sgUserSendMessage();
                ChatMsgManager::sendMessage(e);
            } else {
                if (_pViewItem && _pViewItem->_uid == uid && _pViewItem->_pChatMainWgt) {
                    emit _pViewItem->_pChatMainWgt->sgSendFailed(msgId.data());
                }

                emit g_pMainPanel->sgShowInfoMessageBox(tr("文件上传失败!"));
            }
        });
    }
}

/**
 *
 * @param uid
 * @param chatType
 */
void ChatViewMainPanel::sendShockMessage(const st::entity::UID &uid,
                                         int chatType)
{
    if (g_pMainPanel) {
        // 发送消息
        long long send_time =
            QDateTime::currentDateTime().toMSecsSinceEpoch() - DC.getServerDiffTime() *
            1000;
        string msgId = utils::uuid();
        st::entity::ImMessageInfo message;
        message.ChatType = chatType;
        message.MsgId = msgId;
        message.To = uid.usrId();
        message.RealJid = uid.realId();
        message.XmppId = uid.usrId();
        message.From = g_pMainPanel->getSelfUserId();
        message.LastUpdateTime = send_time;
        message.Type = st::entity::MessageTypeShock;
        message.Content = "[窗口抖动]";
        message.Direction = st::entity::MessageDirectionSent;
        message.UserName = DC.getSelfName();
        S_Message e;
        e.message = message;
        e.userId = uid.usrId();
        e.time = send_time;
        e.chatType = chatType;
        emit g_pMainPanel->sgUserSendMessage();
        ChatMsgManager::sendMessage(e);

        if (!_mapItems.contains(uid)) {
            return;
        }

        auto *pViewItem = _mapItems.get(uid);

        if (pViewItem) {
            pViewItem->showMessage(message, ChatMainWgt::EM_JUM_BOTTOM);
        }
    }
}

void ChatViewMainPanel::sendAudioVideoMessage(const st::entity::UID &uid,
                                              int chatType, bool isVideo, const QString &json)
{
    long long send_time =
        QDateTime::currentDateTime().toMSecsSinceEpoch() - DC.getServerDiffTime() *
        1000;
    string msgId = utils::uuid();
    st::entity::ImMessageInfo message;
    message.ChatType = chatType;
    message.MsgId = msgId;
    message.To = uid.usrId();
    message.RealJid = uid.realId();
    message.XmppId = uid.usrId();
    message.From = g_pMainPanel->getSelfUserId();
    message.LastUpdateTime = send_time;
    message.Content = "当前客户端不支持实时视频";

    if (chatType == st::Enum::TwoPersonChat) {
        if (isVideo) {
            message.Type = st::entity::WebRTC_MsgType_VideoCall;
        } else {
            message.Content = "当前客户端不支持实时音频";
            message.Type = st::entity::WebRTC_MsgType_AudioCall;
        }
    } else if (chatType == st::Enum::GroupChat) {
        message.Type = st::entity::WebRTC_MsgType_Video_Group;
    } else {
        return;
    }

    message.Direction = st::entity::MessageDirectionSent;
    message.UserName = DC.getSelfName();
    message.ExtendedInfo = json.toStdString();
    S_Message e;
    e.message = message;
    e.userId = uid.usrId();
    e.time = send_time;
    e.chatType = chatType;
    emit g_pMainPanel->sgUserSendMessage();
    ChatMsgManager::sendMessage(e);

    if (!_mapItems.contains(uid)) {
        return;
    }

    auto *pViewItem = _mapItems.get(uid);

    if (pViewItem) {
        pViewItem->showMessage(message, ChatMainWgt::EM_JUM_BOTTOM);
    }
}

void ChatViewMainPanel::postInterface(const string &url,
                                      const string &params)
{
    QT_CONCURRENT_FUNC(&ChatMsgManager::postInterface, url, params);
}

void ChatViewMainPanel::sendGetRequest(const string &url)
{
    QT_CONCURRENT_FUNC(&ChatMsgManager::sendGetRequest, url);
}

void ChatViewMainPanel::updateMessageExtendInfo(const string &msgId,
                                                const string &info)
{
    QT_CONCURRENT_FUNC(&ChatMsgManager::updateMessageExtendInfo, msgId, info);
}

//
void ChatViewMainPanel::onRecvWebRtcCommand(int msgType,
                                            const string &peerId, const string &cmd, bool isCarbon)
{
    qDebug() << cmd.data();
    QJsonDocument document = QJsonDocument::fromJson(cmd.data());

    if (document.isNull()) {
    } else {
        auto obj = document.object();
        QString type = obj.value("type").toString();

        if ((type == "create"
             || type ==
             "busy" /*|| type == "canceled" || type == "close" || type == "deny"*/)
            && isCarbon) {
            return;
        }

        if (type == "create") {
#ifdef _WINDOWS
#ifdef PLATFORM_WIN32
            QtMessageBox::information(g_pMainPanel, tr("提醒"),
                                      tr("暂不支持此功能!"));
            return;
#endif
#endif
            //            onRecvVideo(peerId);
            string selfId = DC.getSelfUserId();
            emit sgShowVidioWnd(st::entity::JID(peerId).basename(), selfId,
                                WebRTC_MsgType_VideoCall == msgType);
        } else {
            if (_pAudioVideoManager) {
                if (type == "pickup" && isCarbon) {
                    _pAudioVideoManager->onCloseAudioVideo(peerId);
                }

                if (type == "deny" && isCarbon) {
                    _pAudioVideoManager->onCloseAudioVideo(peerId);
                } else {
                    //
                    _pAudioVideoManager->onRecvWebRtcCommand(peerId.data(), cmd.data());

                    //
                    if ((type == "cancel" || type == "deny" || type == "timeout")
                        && _pAudioVideoManager->isCall(peerId)) {
                        QString njson = QString("{\"type\":\"%1\", \"time\": 0, \"desc\":\"\"}").arg(
                                            type);
                        bool isVideo = _pAudioVideoManager->isVideo(peerId);
                        sendAudioVideoMessage(st::entity::UID(peerId), st::Enum::TwoPersonChat,
                                              isVideo, njson);
                    }
                }
            }
        }
    }
}

void ChatViewMainPanel::start2Talk_old(const string &peerId, bool isVideo,
                                       bool isCall)
{
    if (_pAudioVideoManager) {
        QFuture<string> future = QT_CONCURRENT_FUNC([]() -> string {
            QString url = QString("%1/rtc?username=%2")
            .arg(NavigationManager::instance().getVideoUrl().data())
            .arg(DC.getClientAuthKey().data());
            return ChatMsgManager::sendGetRequest(url.toStdString());
        });

        //        future.waitForFinished();
        while (!future.isFinished()) {
            QApplication::processEvents(QEventLoop::AllEvents, 100);
        }

        string data = future.result();
        _pAudioVideoManager->start2Talk_old(data.data(), peerId, isVideo, isCall);
    }
}

//
void ChatViewMainPanel::onSendSignal(const QString &json, const QString &id)
{
    QT_CONCURRENT_FUNC([this, json, id]() {
        if (_pAudioVideoManager) {
            ChatMsgManager::sendWebRtcCommand(
                _pAudioVideoManager->isVideo(id.toStdString()) ?
                st::entity::WebRTC_MsgType_VideoCall :
                st::entity::WebRTC_MsgType_AudioCall, json.toStdString(), id.toStdString());
        }

        auto document = QJsonDocument::fromJson(json.toUtf8());

        if (document.isNull()) {
        } else {
            auto obj = document.object();
            QString type = obj.value("type").toString();

            if ((type == "cancel" || type == "deny" || type == "timeout")
                && _pAudioVideoManager->isCall(id.toStdString())) {
                QString njson = QString("{\"type\":\"%1\", \"time\": 0, \"desc\":\"\"}").arg(
                                    type);
                bool isVideo = _pAudioVideoManager->isVideo(id.toStdString());
                sendAudioVideoMessage(st::entity::UID(id), st::Enum::TwoPersonChat,
                                      isVideo, njson);
            }
        }
    });
}

//
void ChatViewMainPanel::startGroupTalk(const QString &id, const QString &name)
{
    if (_pAudioVideoManager) {
        _pAudioVideoManager->startGroupTalk(id, name);
    }
}

void ChatViewMainPanel::getUserMedal(const string &xmppId,
                                     std::set<st::StUserMedal> &medal)
{
    if (_user_medals.find(xmppId) != _user_medals.end()) {
        medal = _user_medals[xmppId];
        return;
    }

    ChatMsgManager::getUserMedal(xmppId, medal);
    _user_medals[xmppId] = medal;
}

void ChatViewMainPanel::onUserMadelChanged(const
                                           std::vector<st::entity::ImUserStatusMedal> &userMedals)
{
    std::set<string> changedUser;

    for (const auto &medal : userMedals) {
        string xmppId = medal.userId + "@" + medal.host;

        if (_user_medals.find(xmppId) != _user_medals.end()) {
            auto &curUserMedal = _user_medals[xmppId];
            auto itFind = std::find_if(curUserMedal.begin(),
            curUserMedal.end(), [medal](const st::StUserMedal & m) {
                return m.medalId == medal.medalId;
            });

            if (itFind != curUserMedal.end()) {
                curUserMedal.erase(itFind);
            }

            if (medal.medalStatus != 0) {
                curUserMedal.insert(st::StUserMedal(medal.medalId, medal.medalStatus));
            }

            changedUser.insert(xmppId);
        }
    }

    //
    for (const auto &pItem : _mapItems.values()) {
        //        auto* pItem = it.second;
        if (pItem) {
            if (pItem->_chatType == st::Enum::GroupChat) {
                pItem->_pChatMainWgt->onUserMedalChanged(changedUser);
            } else if (pItem->_chatType == st::Enum::TwoPersonChat &&
                       changedUser.find(pItem->_uid.usrId()) != changedUser.end()) {
                pItem->_pStatusWgt->onUpdateMedal();
            }
        }
    }
}

// show message search window
void ChatViewMainPanel::onShowSearchResult(const QString &key,
                                           const QString &xmppId)
{
    if (nullptr == _pMsgRecordManager) {
        _pMsgRecordManager = new MessageRecordManager(this);
    }

    _pMsgRecordManager->setSearch(key, xmppId);
    _pMsgRecordManager->showCenter(false, this);
}

void ChatViewMainPanel::onShowSearchFileWnd(const QString &key)
{
    if (nullptr == _pFileRecordWnd) {
        _pFileRecordWnd = new FileRecordWnd(this);
    }

    _pFileRecordWnd->setSearch(key);
    _pFileRecordWnd->showCenter(false, this);
}

void ChatViewMainPanel::onRecvLoginProcessMessage(const char *message)
{
    emit sgShowLiteMessageBox(true, message);
}

/**
 *
 */
void ChatViewMainPanel::sendEmoticonMessage(const QString &id,
                                            const QString &messageContent, bool isShowAll, const string &messageId)
{
    if (_pViewItem && id != _pViewItem->conversionId()) {
        return;
    }

    if (_pViewItem && _pViewItem->_pInputWgt && isShowAll) {
        _pViewItem->_pInputWgt->dealFile(messageContent, false);
        return;
    }

    long long send_time =
        QDateTime::currentDateTime().toMSecsSinceEpoch() - DC.getServerDiffTime() *
        1000;
    string msgId = messageId.empty() ? utils::uuid() : messageId;
    entity::ImMessageInfo message;
    message.ChatType = _pViewItem->_chatType;
    message.MsgId = msgId;
    message.To = _pViewItem->_uid.usrId();
    message.RealJid = _pViewItem->_uid.realId();
    message.XmppId = _pViewItem->_uid.usrId();
    message.From = g_pMainPanel->getSelfUserId();
    message.LastUpdateTime = send_time;
    message.Type = entity::MessageTypeImageNew;
    message.Content = messageContent.toStdString();
    message.Direction = entity::MessageDirectionSent;
    message.UserName = DC.getSelfName();
    S_Message e;
    e.message = message;
    e.userId = _pViewItem->_uid.usrId();
    e.time = send_time;
    e.chatType = _pViewItem->_chatType;
    emit g_pMainPanel->sgUserSendMessage();
    ChatMsgManager::sendMessage(e);

    if (_pViewItem) {
        _pViewItem->showMessage(message, ChatMainWgt::EM_JUM_BOTTOM);
    }
}

/**
 *
 * @param id
 * @param messageContent
 */
void ChatViewMainPanel::sendCollectionMessage(const QString &id,
                                              const QString &imagePath, const QString &imgLink)
{
    if (_pViewItem && id != _pViewItem->conversionId()) {
        return;
    }

    if (_pViewItem && _pViewItem->_pInputWgt) {
        _pViewItem->_pInputWgt->dealFile(imagePath, false, imgLink);
    }
}

//
void ChatViewMainPanel::insertAt(const QString &groupId, const QString &userId)
{
    UID uid(groupId);

    if (!_mapItems.contains(uid)) {
        return;
    }

    auto *pViewItem = _mapItems.get(uid);

    if (pViewItem) {
        pViewItem->_pInputWgt->insertAt(userId.toStdString());
    }
}

// 拍一拍
void ChatViewMainPanel::clapSomebody(const QString &groupId,
                                     const QString &userId)
{
    UID uid(groupId);

    if (!_mapItems.contains(uid)) {
        return;
    }

    auto *pViewItem = _mapItems.get(uid);

    if (pViewItem) {
        long long send_time =
            QDateTime::currentDateTime().toMSecsSinceEpoch() - DC.getServerDiffTime() *
            1000;
        string msgId = st::utils::uuid();
        st::entity::ImMessageInfo message;
        S_Message e;
        message.ChatType = pViewItem->_chatType;
        message.To = uid.usrId();
        message.RealJid = uid.realId();
        message.XmppId = uid.usrId();
        e.userId = uid.usrId();
        message.MsgId = msgId;
        message.From = g_pMainPanel->getSelfUserId();
        message.LastUpdateTime = send_time;
        message.Type = st::entity::MessageTypeGroupNotify;
        message.Direction = st::entity::MessageDirectionSent;
        message.UserName = DC.getSelfName();
        string userName{};
        auto userInfo = DB_PLAT.getUserInfo(userId.toStdString());

        if (userInfo) {
            userName = st::getUserNameNoMask(userInfo);
        } else {
            userName = userId.toStdString();
        }

        message.Content = tr("%1 拍了拍 \"%2\"").arg(message.UserName.data()).arg(
                              userName.data()).toStdString();
        // 发送消息
        e.message = message;
        e.chatType = message.ChatType;
        e.time = send_time;
        emit g_pMainPanel->sgUserSendMessage();
        ChatMsgManager::sendMessage(e);
        pViewItem->showMessage(message, ChatMainWgt::EM_JUM_BOTTOM);
    }
}

//
void ChatViewMainPanel::addSending(const string &msgId)
{
    QMutexLocker locker(&_sendingMutex);
    _sending.insert(msgId);
}

void ChatViewMainPanel::eraseSending(const string &msgId)
{
    QMutexLocker locker(&_sendingMutex);

    if (_sending.find(msgId) != _sending.end()) {
        _sending.erase(msgId);
    }
}

bool ChatViewMainPanel::isSending(const string &msgId)
{
    bool isSending = false;
    {
        QMutexLocker locker(&_sendingMutex);
        isSending = _sending.find(msgId) != _sending.end();
    }

    if (isSending) {
        eraseSending(msgId);
    }

    return isSending;
}

//
void ChatViewMainPanel::downloadFileWithProcess(const QString &url,
                                                const QString &path, const QString &key, QWidget *wget)
{
    QPointer<QWidget> wgtPointer(wget);
    auto *file = new QFile(path + ".downloading");

    if (!file->open(QIODevice::ReadWrite)) {
        if (wgtPointer && !wgtPointer.isNull()) {
            auto fileItem = qobject_cast<FileSendReceiveMessItem *>(wgtPointer.data());

            if (fileItem) {
                fileItem->onDownloadFailed();
            }
        }

        emit sgShowLiteMessageBox(false,
                                  tr("写入文件失败, 您可能没有权限写入该文件目录或者您的磁盘已满"));
        return;
    }

    QString nUrl(url);

    if (url.startsWith("file/") || url.startsWith("/file")) {
        nUrl = QString("%1/%2").arg(
                   NavigationManager::instance().getFileHttpHost().data(), url);
    }

    QNetworkRequest req(nUrl);
    req.setRawHeader("keep-alive", "true");

    if (_netManager->networkAccessible() == QNetworkAccessManager::NotAccessible) {
        qInfo() << "net work NotAccessible";
        _netManager->setNetworkAccessible(QNetworkAccessManager::Accessible);
    }

    QNetworkReply *reply = _netManager->get(req);
    reply->ignoreSslErrors();
    qint64 *t = new qint64;
    qint64 *last = new qint64;
    *t = QDateTime::currentMSecsSinceEpoch();
    *last = 0;
    connect(reply, &QNetworkReply::downloadProgress, this, [this, key, t,
          last](qint64 bytesReceived, qint64 bytesTotal) {
        qint64 now = QDateTime::currentMSecsSinceEpoch();

        if (now != *t) {
            FileProcessMessage e;
            e.key = key.toStdString();
            e.downloadTotal = bytesTotal;
            e.downloadNow = bytesReceived;
            e.speed = ((bytesReceived - *last) / (now - *t)) * 1000;
            onRecvFileProcessMessage(e);
        }

        *t = now;
        *last = bytesReceived;
    });
    connect(reply, &QNetworkReply::readyRead, this, [this, wgtPointer, file,
          reply]() {
        auto data = reply->readAll();
        auto len = file->write(data);

        if (len < data.size()) {
            if (wgtPointer && !wgtPointer.isNull()) {
                auto fileItem = qobject_cast<FileSendReceiveMessItem *>(wgtPointer.data());

                if (fileItem) {
                    fileItem->onDownloadFailed();
                }
            }

            emit sgShowLiteMessageBox(false,
                                      tr("写入文件失败, 您可能没有权限写入该文件目录或者您的磁盘已满"));
        } else {
            file->flush();
        }
    });
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), this,
            SLOT(onHttpError(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(sslErrors(const QList<QSslError> &)), this,
            SLOT(onSSLError(const QList<QSslError> &)));
    //
    connect(reply, &QNetworkReply::finished, this, [wgtPointer, reply, file, t,
                                                                last,
                path, key]() {
        auto data = reply->readAll();
        file->write(data);
        file->flush();
        file->close();
        bool success = (reply->error() == QNetworkReply::NoError);

        if (!success) {
            file->remove();
            qCritical() << "download eee error";
        } else {
            file->rename(path);
        }

        file->deleteLater();
        delete t;
        delete last;

        if (wgtPointer && !wgtPointer.isNull()) {
            auto fileItem = qobject_cast<FileSendReceiveMessItem *>(wgtPointer.data());

            if (fileItem) {
                success ? fileItem->downloadOrUploadSuccess() : fileItem->onDownloadFailed();
            } else {
                auto videoItem = qobject_cast<VideoMessageItem *>(wgtPointer.data());

                if (videoItem && success) {
                    videoItem->downloadSuccess();
                }
            }
        }
    });
}

void ChatViewMainPanel::onHttpError(QNetworkReply::NetworkError err)
{
    qInfo() << err;
}

void ChatViewMainPanel::onSSLError(const QList<QSslError> &errors)
{
    qInfo() << errors;
}

//
void ChatViewMainPanel::showDropWnd(const QPixmap &mask, const QString &name)
{
    // init
    if (nullptr == _dropWnd) {
        _dropWnd = new DropWnd;
        _pStackedLayout->addWidget(_dropWnd);
    }

    //
    _pStackedLayout->setCurrentWidget(_dropWnd);
    _dropWnd->setDrop(mask, name);
}

void ChatViewMainPanel::cancelDrop()
{
    if (_pViewItem) {
        _pStackedLayout->setCurrentWidget(_pViewItem);
    } else {
        showEmptyLabel();
    }
}

void ChatViewMainPanel::dragEnterEvent(QDragEnterEvent *e)
{
    if (!_pViewItem) {
        return;
    }

    if (e->mimeData()->hasUrls()) {
        e->setDropAction(Qt::CopyAction);
        e->acceptProposedAction();
        auto pic = this->grab();
        showDropWnd(pic, _pViewItem->_name);
    } else {
        e->ignore();
    }
}

void ChatViewMainPanel::dragMoveEvent(QDragMoveEvent *e)
{
    e->accept();
}

void ChatViewMainPanel::dropEvent(QDropEvent *e)
{
    if (_pViewItem) {
        _pViewItem->_pInputWgt->dealMimeData(e->mimeData());
        e->acceptProposedAction();
        g_pMainPanel->cancelDrop();
    }
}

void ChatViewMainPanel::dragLeaveEvent(QDragLeaveEvent *e)
{
    cancelDrop();
    e->accept();
}

void ChatViewMainPanel::onShowQRcodeWnd()
{
    if (nullptr == _pQRcode) {
        _pQRcode = new QRcode(this);
    } else if (_pQRcode->isVisible()) {
        _pQRcode->setVisible(false);
    }

    _pQRcode->showCenter(false, this);
}

void ChatViewMainPanel::onForbiddenWordGroupStateMsg(const QString &groupId,
                                                     bool status,
                                                     bool showMessage)
{
    UID uid(groupId);

    if (_mapItems.contains(uid) && _mapItems.get(uid)) {
        auto *item = _mapItems.get(uid);

        if (item && item->_pInputWgt) {
            item->setForbiddenWordState(status, showMessage);
        }
    }
}
