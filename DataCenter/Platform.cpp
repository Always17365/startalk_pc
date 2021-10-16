#include "Platform.h"

#include <time.h>
#include <string.h>
#include <sstream>
#include <algorithm>
#include "dbPlatForm.h"

//#include <cctype>
#include "Util/Entity/JID.h"
#include <utility>
#include "Util/utils.h"

#ifdef _LINUX
    #include <algorithm>
#endif

using std::string;
using std::istringstream;

DataCenter &DataCenter::instance()
{
    static DataCenter platform;
    return platform;
}

void DataCenter::setExecutePath(const std::string &exePath)
{
    executePath = exePath;
}

std::string DataCenter::getExecutePath()
{
    return executePath;
}

void DataCenter::setExecuteName(const std::string &name)
{
    executeName = name;
}

std::string DataCenter::getExecuteName()
{
    return executeName;
}

/**
  * @函数名
  * @功能描述 获取程序appdata Roaming 下路径
  * @参数
  * @date 2018.9.17
  */
std::string DataCenter::getAppdataRoamingPath() const
{
    return AppSetting::instance().getUserDirectory();
}

/**
  * @函数名
  * @功能描述 设置程序appdata Roaming 下路径
  * @参数
  * @date 2018.9.17
  */
void DataCenter::setAppdataRoamingPath(const std::string &path)
{
    AppSetting::instance().setUserDirectory(path);
}

std::string DataCenter::getAppdataRoamingUserPath() const
{
    std::ostringstream userPath;
    userPath << AppSetting::instance().getUserDirectory()
             << "/"
             << _strUserId
             << "@"
             << _strDomain
             << "_"
             << _navName;
    return userPath.str();
}

std::string DataCenter::getLocalEmoticonPath(const std::string &packegId) const
{
    return AppSetting::instance().getUserDirectory() + "/emoticon/" + packegId;
}

std::string DataCenter::getLocalEmoticonPacketPath(const std::string &packegId)
const
{
    return AppSetting::instance().getUserDirectory() + "/emoticon/packet/" +
           packegId;
}

std::string DataCenter::getEmoticonIconPath() const
{
    return AppSetting::instance().getUserDirectory() + "/emoticon/icon";
}

std::string DataCenter::getTempEmoticonPath(const std::string &packegId)
{
    return AppSetting::instance().getUserDirectory() + "/emoticon/temp/" + packegId;
}

/**
  * @函数名
  * @功能描述 默认临时下载文件目录
  * @参数
  * @date 2018.10.22
  */
std::string DataCenter::getTempFilePath() const
{
    return getAppdataRoamingUserPath() + "/temp";
}

/**
  * @函数名   getSelfUserId
  * @功能描述 获取自身id
  * @参数
  * @author   cc
  * @date     2018/09/19
  */
std::string DataCenter::getSelfUserId() const
{
    return _strUserId;
}

std::string DataCenter::getSelfXmppId() const
{
    if (_strUserId.empty() || _strDomain.empty()) {
        return std::string();
    }

    std::ostringstream xmppId;
    xmppId << _strUserId
           << "@"
           << _strDomain;
    return xmppId.str();
}

/**
  * @函数名   setSelfUserId
  * @功能描述 设置自身id
  * @参数
  * @author   cc
  * @date     2018/09/19
  */
void DataCenter::setSelfUserId(const std::string &selfUserId)
{
    _strUserId = selfUserId;
}

std::string DataCenter::getSelfDomain() const
{
    return this->_strDomain;
}

void DataCenter::setSelfDomain(const std::string &selfDomain)
{
    this->_strDomain = selfDomain;
}

/**
 *
 * @return 自己名称
 */
std::string DataCenter::getSelfName()
{
    if (_strSelfName.empty()) {
        std::shared_ptr<st::entity::ImUserInfo> info = DB_PLAT.getUserInfo(
                                                           _strUserId + "@" + _strDomain);

        if (nullptr != info) {
            _strSelfName = st::getUserNameNoMask(info);
        }
    }

    return _strSelfName;
}

long long DataCenter::getServerDiffTime()
{
    return this->_serverTimeDiff;
}

void DataCenter::setServerDiffTime(long long serverDiffTime)
{
    this->_serverTimeDiff = serverDiffTime;
}

std::string DataCenter::getServerAuthKey()
{
    std::lock_guard<st::util::spin_mutex> lock(cKeySm);
    return this->_serverAuthKey;
}

void DataCenter::setServerAuthKey(const std::string &authKey)
{
    std::lock_guard<st::util::spin_mutex> lock(cKeySm);
    this->_serverAuthKey = authKey;
}

std::string DataCenter::getClientAuthKey()
{
    std::lock_guard<st::util::spin_mutex> lock(cKeySm);
    time_t now = time(nullptr);
    long long time = now - this->_serverTimeDiff;
    std::string key = st::utils::getStrMd5(_serverAuthKey + std::to_string(time));

    if (key.empty()) {
        throw std::runtime_error("getClientAuthKey failed: key is empty");
    }

    std::transform(key.begin(), key.end(), key.begin(), ::toupper);
    std::stringstream ss;
    ss << "u=" << _strUserId
       << "&k=" << key
       << "&t=" << time
       << "&d=" << _strDomain
       << "&r=" << _strResource;
    std::string content = ss.str();
    return st::utils::base64_encode((unsigned char *) content.c_str(),
                                    (unsigned int) content.length());;
}

void DataCenter::setClientAuthKey(const std::string &clientAuthKey)
{
    this->_q_ckey = clientAuthKey;
}

std::string DataCenter::getPlatformStr() const
{
#ifdef _WINDOWS
#ifdef _WIN64
    return "PC64";
#else
    return "PC32";
#endif // _WIN32
#else
#ifdef _LINUX
    return "LINUX";
#else
    return "Mac";
#endif
#endif
}

std::string DataCenter::getClientVersion() const
{
    return std::to_string(GLOBAL_INTERNAL_VERSION);
}

long long DataCenter::getClientNumVerison() const
{
    return GLOBAL_INTERNAL_VERSION;
}

std::string DataCenter::get_build_date_time() const
{
    std::ostringstream src;
    src << __TIME__ << " " << __DATE__;
    return src.str();
}

/**
  * @函数名
  * @功能描述
  * @参数
  * @author cc
  * @date 2018.10.12
  */
bool DataCenter::isOnline(const std::string &xmppId)
{
    bool isOnline = false;
    {
        std::lock_guard<st::util::spin_mutex> lock(sm);

        if (_mapUserStatus.contains(xmppId)) {
            isOnline = _mapUserStatus.get(xmppId) != "offline";
            _mapUserStatus.update(xmppId);
        } else {
            _mapUserStatus.insert(xmppId, "offline");
        }
    }
    return isOnline;
}

//
std::string DataCenter::getUserStatus(const std::string &userId)
{
    std::string status = "offline";
    {
        std::lock_guard<st::util::spin_mutex> lock(sm);

        if (_mapUserStatus.contains(userId)) {
            status = _mapUserStatus.get(userId);
            _mapUserStatus.update(userId);
        } else {
            _mapUserStatus.insert(userId, "offline");
        }
    }
    return status;
}

/**
 *
 */
void DataCenter::loadOnlineData(const std::map<std::string, std::string>
                                &userStatus)
{
    std::lock_guard<st::util::spin_mutex> lock(sm);

    for (const auto &state : userStatus) {
        _mapUserStatus.insert(state.first, state.second);
    }
}

std::vector<std::string> DataCenter::getInterestUsers()
{
    return _mapUserStatus.keys();
}

std::string DataCenter::getConfigPath() const
{
    if (!AppSetting::instance().getUserDirectory().empty()) {
        return AppSetting::instance().getUserDirectory() + "/config";
    }

    return std::string();
}

std::string DataCenter::getHistoryDir() const
{
    return _strHistoryFileDir;
}

void DataCenter::setHistoryDir(const std::string &dir)
{
    _strHistoryFileDir = dir;
}

std::string DataCenter::getSelfResource() const
{
    return _strResource;
}

void DataCenter::setSelfResource(const std::string &resource)
{
    _strResource = resource;
}

int DataCenter::getDbVersion()
{
    return DB_VERSION;
}

void DataCenter::setMainThreadId()
{
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    _mainThreadId = oss.str();
}

std::string DataCenter::getMainThreadId()
{
    return _mainThreadId;
}

bool DataCenter::isMainThread()
{
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::string tid = oss.str();
    return tid == _mainThreadId;
}


void DataCenter::setNavName(const std::string &name)
{
    _navName = name;
}


/**
 *
 */
void DataCenter::setAppNetVersion(long long version)
{
    APPLICATION_NET_VERSION = version;
}

/**
 *
 */
long long DataCenter::getAppNetVersion()
{
    return APPLICATION_NET_VERSION;
}

/**
 *
 */
void DataCenter::setBetaUrl(const std::string &url)
{
    beatUrl = url;
}

std::string DataCenter::getBetaUrl()
{
    return beatUrl;
}

void DataCenter::setSystemInfo(const std::string &sys)
{
    sys_str = sys;
}

std::string DataCenter::getSystemInfo()
{
    return sys_str;
}

void DataCenter::setLoginNav(const std::string &nav)
{
    loginNav = nav;
}

void DataCenter::setQvt(const std::string &qvt)
{
    _qvt = qvt;
}

std::string DataCenter::getQvt()
{
    return _qvt;
}

void DataCenter::setSeats(const std::string &seats)
{
    _seats = seats;
}

std::string DataCenter::getSeats()
{
    return _seats;
}

std::string DataCenter::getLoginNav()
{
    return loginNav;
}

bool DataCenter::isForbiddenWordGroup(const std::string& groupId) {
    return _forbiddenWordGroups.find(groupId) != _forbiddenWordGroups.end();
}

void DataCenter::addForbiddenWordGroup(const std::string& groupId) {
    _forbiddenWordGroups.insert(groupId);
}

void DataCenter::removeForbiddenWordGroup(const std::string& groupId) {
    if (_forbiddenWordGroups.find(groupId) != _forbiddenWordGroups.end()) {
        _forbiddenWordGroups.erase(groupId);
    }
}


namespace st {
/**
  * @函数名   GetFileNameByUrl
  * @功能描述 根据url截取文件名
  * @参数
  * @author   cc
  * @date     2018/10/08
  */
std::string GetFileNameByUrl(const std::string &url)
{
    if (url.empty()) {
        return std::string();
    }

    std::ostringstream fileName;
    std::string rurl(url);

    fileName << utils::getStrMd5(rurl)
             << "."
             << st::utils::getFileSuffix(url);
    return fileName.str();
}

/**
  * @函数名
  * @功能描述
  * @参数
  * @author   cc
  * @date     2018/10/09
  */
std::string GetHeadPathByUrl(const std::string &url)
{
    std::ostringstream src;
    src << DC.getAppdataRoamingUserPath()
        << "/image/headphoto/"
        << GetFileNameByUrl(url);
    return src.str();
}

/**
  * @函数名
  * @功能描述
  * @参数
  * @author   cc
  * @date     2018/10/16
  */
std::string GetFilePathByUrl(const std::string &url)
{
    std::ostringstream src;
    src << AppSetting::instance().getFileSaveDirectory()
        << "/"
        << GetFileNameByUrl(url);
    return src.str();
}
std::string GetImagePathByUrl(const std::string &url)
{
    std::ostringstream src;
    src << DC.getAppdataRoamingUserPath()
        << "/image/temp/"
        << GetFileNameByUrl(url);
    return src.str();
}

std::string GetSrcImagePathByUrl(const std::string &url)
{
    std::ostringstream src;
    src << DC.getAppdataRoamingUserPath()
        << "/image/source/"
        << GetFileNameByUrl(url);
    return src.str();
}

std::string getCollectionPath(const std::string &netPath)
{
    std::ostringstream src;
    src << DC.getAppdataRoamingUserPath()
        << "/emoticon/collection/"
        << GetFileNameByUrl(netPath);
    return src.str();
}

std::string getOAIconPath(const std::string &netPath)
{
    std::ostringstream src;
    src << DC.getAppdataRoamingUserPath()
        << "/oaIcon/"
        << GetFileNameByUrl(netPath);
    return src.str();
}

std::string getUserName(const std::shared_ptr<st::entity::ImUserInfo> &userInfo)
{
    std::string ret;

    if (userInfo) {
        ret = DB_PLAT.getMaskName(userInfo->XmppId);

        if (ret.empty()) {
            ret = userInfo->NickName;
        }

        if (ret.empty()) {
            ret = userInfo->Name;
        }

        if (ret.empty()) {
            ret = st::entity::JID(userInfo->XmppId).username();
        }
    }

    return ret;
}

std::string getUserName(const std::string &xmppId)
{
    std::string ret;

    if (!xmppId.empty()) {
        auto info = DB_PLAT.getUserInfo(xmppId);
        ret = getUserName(info);

        if (ret.empty()) {
            ret = st::entity::JID(xmppId).username();
        }
    }

    return ret;
}

std::string getUserNameNoMask(const std::shared_ptr<st::entity::ImUserInfo>
                              &userInfo)
{
    std::string ret;

    if (userInfo) {
        ret = userInfo->NickName;

        if (ret.empty()) {
            ret = userInfo->Name;
        }

        if (ret.empty()) {
            ret = st::entity::JID(userInfo->XmppId).username();
        }
    }

    return ret;
}

std::string getUserNameNoMask(const std::string &xmppId)
{
    std::string ret;

    if (!xmppId.empty()) {
        auto info = DB_PLAT.getUserInfo(xmppId);
        ret = getUserNameNoMask(info);

        if (ret.empty()) {
            ret = st::entity::JID(xmppId).username();
        }
    }

    return ret;
}

std::string getVideoPathByUrl(const std::string &url)
{
    //
    std::ostringstream src;
    src << DC.getAppdataRoamingUserPath()
        << "/video/"
        << GetFileNameByUrl(url);
    return src.str();
}

std::string getGroupName(const std::shared_ptr<st::entity::ImGroupInfo>
                         &groupInfo)
{
    std::string ret;

    if (groupInfo) {
        ret = groupInfo->Name;

        if (ret.empty()) {
            ret = st::entity::JID(groupInfo->GroupId).username();
        }
    }

    return ret;
}

std::string getGroupName(const std::string &xmppId)
{
    std::string ret;

    if (!xmppId.empty()) {
        auto info = DB_PLAT.getGroupInfo(xmppId);
        ret = getGroupName(info);
    }

    return ret;
}

std::string getMedalPath(const std::string &link)
{
    std::ostringstream src;
    src << DC.getAppdataRoamingUserPath()
        << "/image/medal/"
        << GetFileNameByUrl(link);
    return src.str();
}
}

time_t build_time()
{
    string datestr = __DATE__;
    string timestr = __TIME__;
    istringstream iss_date( datestr );
    string str_month;
    int day;
    int year;
    iss_date >> str_month >> day >> year;
    int month;

    if     ( str_month == "Jan" ) {
        month = 1;
    } else if ( str_month == "Feb" ) {
        month = 2;
    } else if ( str_month == "Mar" ) {
        month = 3;
    } else if ( str_month == "Apr" ) {
        month = 4;
    } else if ( str_month == "May" ) {
        month = 5;
    } else if ( str_month == "Jun" ) {
        month = 6;
    } else if ( str_month == "Jul" ) {
        month = 7;
    } else if ( str_month == "Aug" ) {
        month = 8;
    } else if ( str_month == "Sep" ) {
        month = 9;
    } else if ( str_month == "Oct" ) {
        month = 10;
    } else if ( str_month == "Nov" ) {
        month = 11;
    } else if ( str_month == "Dec" ) {
        month = 12;
    } else {
        exit(-1);
    }

    for ( string::size_type pos = timestr.find( ':' ); pos != string::npos;
          pos = timestr.find( ':', pos ) ) {
        timestr[ pos ] = ' ';
    }

    istringstream iss_time( timestr );
    int hour, min, sec;
    iss_time >> hour >> min >> sec;
    tm t = {0};
    t.tm_mon = month - 1;
    t.tm_mday = day;
    t.tm_year = year - 1900;
    t.tm_hour = hour - 1;
    t.tm_min = min;
    t.tm_sec = sec;
    return mktime(&t);
}
