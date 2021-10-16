#ifndef PLATFORM_H
#define PLATFORM_H

#include "platform_global.h"
#include <string>
#include "entity/im_user.h"
#include "entity/im_group.h"
#include "Util/Spinlock.h"
#include "AppSetting.h"

#include <map>
#include "interface/logic/IDatabasePlug.h"
#include "LogicManager/LogicManager.h"
#include "Util/FrequencyMap.h"
#include "SystemDefine.h"
#include <set>
#include <thread>

#define DC DataCenter::instance()

/**
 */
class PLATFORMSHARED_EXPORT DataCenter
{

private:
    DataCenter() = default;

public:
    void setExecutePath(const std::string &exePath);
    std::string getExecutePath();
    void setExecuteName(const std::string &name);
    std::string getExecuteName();

public:
    std::string getAppdataRoamingPath() const;

    void setAppdataRoamingPath(const std::string &path);

    void setNavName(const std::string &name);

    std::string getAppdataRoamingUserPath() const;

    std::string getLocalEmoticonPath(const std::string &packegId) const;

    std::string getLocalEmoticonPacketPath(const std::string &packegId) const;

    std::string getEmoticonIconPath() const;

    static std::string getTempEmoticonPath(const std::string &packegId);

    std::string getConfigPath() const;

    int getDbVersion();

    std::string getTempFilePath() const;

    std::string getSelfUserId() const;

    std::string getSelfXmppId() const;

    void setSelfUserId(const std::string &selfUserId);

    std::string getSelfDomain() const;

    void setSelfDomain(const std::string &selfDomain);

    std::string getSelfName();

    std::string getSelfResource() const;

    void setSelfResource(const std::string &resource);

    long long getServerDiffTime();

    void setServerDiffTime(long long serverDiffTime);

    std::string getServerAuthKey();

    void setServerAuthKey(const std::string &authKey);

    std::string getClientAuthKey();

    void setClientAuthKey(const std::string &clientAuthKey);

    std::string getPlatformStr() const;

    void setOSInfo(std::string os) {osStr = std::move(os);}
    std::string getOSInfo() { return osStr;}

    void setOSProductType(std::string productType) {osProductType = std::move(productType);}
    std::string getOSProductType() { return osProductType;}

    void setOSVersion(std::string version) {osVersion = std::move(version);}
    std::string getOSVersion() { return osVersion;}

    std::string getClientVersion() const;

    long long getClientNumVerison() const;

    std::string get_build_date_time() const;

    // 历史打开文件夹的路径 全部都是主线程调用
    std::string getHistoryDir() const;

    void setHistoryDir(const std::string &dir);

    bool isOnline(const std::string &xmppId);

    std::string getUserStatus(const std::string &userId);
    //
    void loadOnlineData(const std::map<std::string, std::string> &userStatus);
    //
    std::vector<std::string> getInterestUsers();
    //
    std::string getGlobalVersion() { return APPLICATION_VERSION; };
    //
    void setLoginNav(const std::string &nav);
    std::string getLoginNav();

    inline void setProcessId(int id) { processId = id; };
    inline int getProcessId() { return processId; };

    inline void setShowStaff(bool show) {_showStaff = show; }
    inline bool getShowStaff() { return _showStaff; }

    bool isForbiddenWordGroup(const std::string& groupId);
    void addForbiddenWordGroup(const std::string& groupId);
    void removeForbiddenWordGroup(const std::string& groupId);

public:
    void setAppNetVersion(long long version);
    long long getAppNetVersion();
    void setBetaUrl(const std::string &url);
    std::string getBetaUrl();

public:
    void setSystemInfo(const std::string &sys);
    std::string getSystemInfo();

public:
    // 主线程Id
    void setMainThreadId();
    std::string getMainThreadId();
    bool isMainThread();

public:
    void setQvt(const std::string &qvt);
    std::string getQvt();
    void setSeats(const std::string &seats);
    std::string getSeats();

private:
    std::string _navName;
    std::string _strUserId;
    std::string _strDomain;
    std::string _strSelfName;
    std::string _strResource;
    std::string _strHistoryFileDir; // 保存历史打开文件夹的路径

    long long _serverTimeDiff{}; // 本机时间与服务器时间差值
    std::string _serverAuthKey;  // 服务端认证登录有消息的Key
    std::string _q_ckey;
    std::string _mainThreadId;   // 主线程id

    FrequencyMap<std::string, std::string> _mapUserStatus {5000};

    st::util::spin_mutex sm;
    st::util::spin_mutex cKeySm;
    std::string beatUrl;
    int APPLICATION_NET_VERSION = 0;
    std::string loginNav;

    int processId{};

    std::string osStr;
    std::string osProductType;
    std::string osVersion;

    std::set<std::string> _forbiddenWordGroups;

private:
    std::string sys_str;
    std::string executePath;
    std::string executeName;

private:
    std::string _qvt;
    std::string _seats;

private:
    bool        _showStaff{true};

public:
    static DataCenter &instance();
};

namespace st {
std::string PLATFORMSHARED_EXPORT GetFileNameByUrl(const std::string &url);
std::string PLATFORMSHARED_EXPORT GetHeadPathByUrl(const std::string &url);
std::string PLATFORMSHARED_EXPORT GetFilePathByUrl(const std::string &url);
std::string PLATFORMSHARED_EXPORT GetImagePathByUrl(const std::string &url);
std::string PLATFORMSHARED_EXPORT GetSrcImagePathByUrl(const std::string &url);
std::string PLATFORMSHARED_EXPORT getMedalPath(const std::string &link);
std::string PLATFORMSHARED_EXPORT getCollectionPath(const std::string &netPath);
std::string PLATFORMSHARED_EXPORT getOAIconPath(const std::string &netPath);
std::string PLATFORMSHARED_EXPORT getUserName(const
                                              std::shared_ptr<st::entity::ImUserInfo> &userInfo);
std::string PLATFORMSHARED_EXPORT getUserName(const std::string &xmppId);
std::string PLATFORMSHARED_EXPORT getUserNameNoMask(const
                                                    std::shared_ptr<st::entity::ImUserInfo> &userInfo);
std::string PLATFORMSHARED_EXPORT getUserNameNoMask(const std::string &xmppId);
std::string PLATFORMSHARED_EXPORT getGroupName(const
                                               std::shared_ptr<st::entity::ImGroupInfo> &groupInfo);
std::string PLATFORMSHARED_EXPORT getGroupName(const std::string &xmppId);
std::string PLATFORMSHARED_EXPORT getVideoPathByUrl(const std::string &url);
}

#endif // PLATFORM_H
