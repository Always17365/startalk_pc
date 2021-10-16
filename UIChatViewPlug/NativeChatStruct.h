#ifndef _NATIVECHATSTRUCT_H_
#define _NATIVECHATSTRUCT_H_

#include <QColor>
#include <QFont>
#include <QMap>
#include <QString>
#include "include/im_enum.h"
#include "include/CommonDefine.h"


class QVBoxLayout;
class StatusWgt;
class ChatMainWgt;
class GroupChatSidebar;
class ToolWgt;
class InputWgt;
struct StSubWgt
{
	QVBoxLayout*      layout;

	StatusWgt*        statusWgt;
	ChatMainWgt*      chatMainWgt;
	GroupChatSidebar* groupSidebar;
	ToolWgt*          toolWgt;
	InputWgt*         inputWgt;

	StSubWgt()
		: layout(nullptr), statusWgt(nullptr), chatMainWgt(nullptr)
		, groupSidebar(nullptr), toolWgt(nullptr), inputWgt(nullptr)
	{

	}
};

enum {
    Type_Invalid,
    Type_Text,
    Type_Image,
    Type_At,
    Type_Url,
};

struct StTextMessage
{
    enum Type {EM_TINVALID, EM_TEXT, EM_IMAGE, EM_EMOTICON, EM_LINK, EM_ATMSG};

    QString content;
    Type    type;
    QString imageLink;
    QString pkgid;
    QString shortCut;
    double  imageWidth{};
    double  imageHeight{};

    explicit StTextMessage(Type t = EM_TINVALID) : type(t){}
};

struct StFileMessage {

    QString fileName;
    QString fileSize;
    QString fileMd5;
    QString fileLink;
    QString fileIcon;

};

struct StCommonTrdMessage {
    QString title;
    QString desc;
    QString img;
    bool    showAll{};
    QString link;
};

struct StCodeMessage {
    QString code;
    QString language;
    QString codeStyle;
};

struct StVideoMessage {
    QString thumbUrl;
    QString videoUrl;
    qreal width {};
    qreal height {};
    bool newVideo{};
};


struct StNetMessageResult {
    QString     msg_id{};
    int         msg_type{};
    int         type{};    //chat type
    int         direction{};
    int         state{}; // message state (0 unSend 1 send)
    int         read_flag{}; // 0 default 1 send 2 read
    qint64      time;
    QString     from;
    QString     to;
    QString     body;
    QString     extend_info;
    QString     backup_info;
    QString     xmpp_id;
    QString     real_id;

    QString     user_name;
    QString     user_head;

    // text message
    std::vector<StTextMessage> text_messages;
    // file info
    StFileMessage file_info;
    // common_trd
    StCommonTrdMessage common_trd;
    // code
    StCodeMessage code;
    // video
    StVideoMessage video;
    //
    std::map<QString, QString> at_users; // id, name
};
Q_DECLARE_METATYPE(StNetMessageResult)

#endif//_NATIVECHATSTRUCT_H_
