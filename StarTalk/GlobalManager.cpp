#include "GlobalManager.h"

#include <QFile>
#include <QJsonDocument>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QJsonArray>
#include "DataCenter/Platform.h"
#include "Util/Log.h"
#include "Util/ui/StyleDefine.h"
#include "Util/ui/qconfig/qconfig.h"

#define DEFAULT_PluginManagerPath ":/QTalk/config/pluginManager.json"
#define DEFAULT_PluginPath "."

#define USER_FOLDER "USER"
#define FILE_FOLDER "FILE"
#define HISTORYU_FOLDER "HISTORY"
#define LOG_LEVEL "LOG_LEVEL"
#define THEME "THEME"
#define FONT "FONT"
#define FONT_LEVEL "FONT_LEVEL"
#define LANGUAGE "LANGUAGE"
#define CHECK_UPDATER "CHECK_UPDATER"
#define UPDATER_VERSION "UPDATER_VERSION"
#define SSL "SSL"

GlobalManager *GlobalManager::instance()
{
    static GlobalManager manager;
    return &manager;
}

/**
  * @功能描述
  * @参数
  * @date 2018.9.17
  */
QJsonDocument GlobalManager::loadJsonConfig(const QString &path)
{
    if (path.isEmpty()) {
        return QJsonDocument();
    }

    QFile file(path);

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QJsonParseError err{};
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);

        if (err.error == QJsonParseError::NoError) {
            return doc;
        } else {
            qWarning() << "load json fail" << path;
            return QJsonDocument();
        }
    } else {
        qWarning() << "open json file fail" << path;
        return QJsonDocument();
    }
}


/**
  * @功能描述
  * @参数
  * @date 2018.9.17
  */
QObject *GlobalManager::getPluginInstanceQt(const QString &key)
{
    return _pluginManager.GetPluginInstanceQt(key);
}

/**
  * @函数名
  * @功能描述
  * @参数
  * @date 2018.9.27
  */
std::shared_ptr<QMap<QString, QObject *> >
GlobalManager::getAllPluginInstanceQt()
{
    return _pluginManager.GetAllPluginInstanceQt();
}

/**
  * @功能描述
  * @参数
  * @date 2018.9.17
  */
void GlobalManager::init()
{
    // init setting
    auto tempAppDatePath = QStandardPaths::writableLocation(
                               QStandardPaths::AppDataLocation).toLocal8Bit();
    auto tempDownloadPath = QStandardPaths::writableLocation(
                                QStandardPaths::DownloadLocation).toLocal8Bit();
    QString configDataDir = QString::fromLocal8Bit(tempAppDatePath);
    QString fileSavePath = tempDownloadPath;
    std::string userPath, historyPath;
    //    int logLevel = st::logger::LEVEL_INVALID;
    int language = 0;
    bool ssl = true;
    _font_level = AppSetting::FONT_LEVEL_NORMAL;

    _pSystemConfig = new st::ConfigLoader(std::string(tempAppDatePath.data()) + "/sysconfig");

    if (_pSystemConfig->load()) {
        userPath = _pSystemConfig->getString(USER_FOLDER);
        fileSavePath = _pSystemConfig->getString(FILE_FOLDER).data();
        historyPath = _pSystemConfig->getString(HISTORYU_FOLDER);
        _theme = _pSystemConfig->getInteger(THEME);
        _font = _pSystemConfig->getString(FONT);
        _font_level = _pSystemConfig->getInteger(FONT_LEVEL);
        language = _pSystemConfig->getInteger(LANGUAGE);
        _updater_version = _pSystemConfig->getInteger(UPDATER_VERSION);

        if (_pSystemConfig->hasKey(SSL)) {
            ssl = _pSystemConfig->getBool(SSL);
        }
    } else {
        error_log("error load sysconf");
    }

    AppSetting::instance().setLanguage(language);

    //
    if (_theme == 0) {
        _theme = 1;
    }

    AppSetting::instance().setThemeMode(_theme);
    //
    AppSetting::instance().setFont(_font);

    //    AppSetting::instance().setLogLevel(logLevel);
    AppSetting::instance().setFontLevel(_font_level);
    AppSetting::instance().with_ssl = ssl;

    int channel = _pSystemConfig->getInteger("CHANNEL");

    if (0 == channel) {
        channel = 1;    // product
    }

    AppSetting::instance().setTestchannel(channel);

    if (!userPath.empty()) {
        configDataDir = QString::fromStdString(userPath);
    }

    if (fileSavePath.isEmpty()) {
        fileSavePath = QString::fromLocal8Bit(tempDownloadPath);
    }

    // 创建文件夹
    QDir dir(configDataDir);

    if (!dir.exists()) {
        bool isOK = dir.mkpath(configDataDir);//创建多级目录

        if (!isOK) {
            error_log("init user folder error {0}", userPath);
            configDataDir = QString::fromLocal8Bit(tempAppDatePath);
        }
    } else if (!QFileInfo(configDataDir).permission(QFileDevice::WriteUser)) {
        configDataDir = QString::fromLocal8Bit(tempAppDatePath);
    }

    dir = QDir(fileSavePath);

    if (!dir.exists()) {
        bool isOK = dir.mkpath(fileSavePath);//创建多级目录

        if (!isOK) {
            error_log("init file folder error {0}", fileSavePath.toStdString());
            fileSavePath = tempDownloadPath;
        }
    } else if (!QFileInfo(fileSavePath).permission(QFileDevice::WriteUser)) {
        fileSavePath = tempDownloadPath;
    }

    // 设置全局路径
    DC.setAppdataRoamingPath(configDataDir.toStdString());
    AppSetting::instance().setFileSaveDirectory(fileSavePath.toStdString());

    //
    if (historyPath.empty()) {
        historyPath = QStandardPaths::writableLocation(
                          QStandardPaths::DownloadLocation).toStdString();
    }

    DC.setHistoryDir(historyPath);
    // 保存最新配置
    saveSysConfig();
    //
    InitPluginManager();
    //
    initThemeConfig();
}

void GlobalManager::saveSysConfig()
{
    std::string hispath = DC.getHistoryDir();
    std::string userpath = DC.getAppdataRoamingPath();
    std::string filepath = AppSetting::instance().getFileSaveDirectory();

    //    int theme = AppSetting::instance().getThemeMode();
    std::string font = AppSetting::instance().getFont();

    //    int font_level = AppSetting::instance().getFontLevel();
    if (_pSystemConfig) {
        _pSystemConfig->setString(FILE_FOLDER, filepath);
        _pSystemConfig->setString(USER_FOLDER, userpath);
        _pSystemConfig->setString(HISTORYU_FOLDER, hispath);
        _pSystemConfig->setInteger("CHANNEL", AppSetting::instance().getTestchannel());
        //        _pSystemConfig->setInteger(LOG_LEVEL, AppSetting::instance().getLogLevel());
        _pSystemConfig->setInteger(THEME, AppSetting::instance().getThemeMode());
        _pSystemConfig->setInteger(LANGUAGE, AppSetting::instance().getLanguage());
        _pSystemConfig->setInteger(FONT_LEVEL, AppSetting::instance().getFontLevel());
        _pSystemConfig->setString(FONT, font);
        _pSystemConfig->saveConfig();
    }
}

/**
  * @功能描述 初始化插件管理器
  * @参数
  * @date 2018.9.17
  */
void GlobalManager::InitPluginManager()
{
    QJsonDocument doc = loadJsonConfig(DEFAULT_PluginManagerPath);

    if (!doc.isNull() && doc.isArray()) {

        QVector<QString> arPlugs;
        auto allPlug  = doc.array();

        for (const auto &plug : allPlug) {
            arPlugs.push_back(plug.toString());
        }

        //
        _pluginManager.setPlugNames(arPlugs);
    }

    _pluginManager.setPluginPath(DEFAULT_PluginPath);
    _pluginManager.LoadPluginAllQt();
    qInfo() << "InitPluginManager done.";
}

/**
  * @功能描述 设置所有样式
  * @参数
  * @date 2018.9.17
  */
void GlobalManager::setStyleSheetAll()
{
    _pstyleSheetManager.setStyleSheets(_theme, _font);
}

/**
  * @功能描述 设置公共样式
  * @参数
  * @date 2018.9.17
  */
void GlobalManager::setStylesForApp()
{
    _pstyleSheetManager.setStylesForApp(_theme, _font);
}

/**
  * @功能描述 设置相应插件样式
  * @参数
  * @date 2018.9.17
  */
void GlobalManager::setStyleSheetForPlugin(const QString &plgName)
{
    _pstyleSheetManager.setStyleSheetForPlugin(plgName, _theme);
}

GlobalManager::GlobalManager()
{
    _pSystemConfig = nullptr;
    init();
}

bool GlobalManager::UnloadPluginQt(const QString &key)
{
    return _pluginManager.UnloadPluginQt(key);
}

/**
 *
 */
void GlobalManager::initThemeConfig()
{
    QString configPath = QString(":/style/style%1/data.xml").arg(_theme);
    auto *config = new st::StConfig;
    st::qConfig::loadConfig(configPath, false, config);

    if ("Style" == config->tagName) {
        for (const auto it : config->children) {
            QString tagName = it->tagName;
            QString value = it->tagVal;

            int r = 255, g = 255, b = 255, a = 255;

            QStringList rgba = value.split(",");

            if (rgba.size() >= 4) {
                r = rgba[0].trimmed().toInt();
                g = rgba[1].trimmed().toInt();
                b = rgba[2].trimmed().toInt();
                a = rgba[3].trimmed().toInt();
            } else {
                error_log("error color {0} {1}", tagName.toStdString(), value.toStdString());
                continue;
            }

            if ("_nav_normal_color" == tagName) {
                st::StyleDefine::instance().setNavNormalColor(QColor(r, g, b, a));
            } else if ("_main_windows_color" == tagName) {
                st::StyleDefine::instance().setMainWindowColor(QColor(r, g, b, a));
            } else if ("_nav_select_color" == tagName) {
                st::StyleDefine::instance().setNavSelectColor(QColor(r, g, b, a));
            } else if ("_search_normal_color" == tagName) {
                st::StyleDefine::instance().setSearchNormalColor(QColor(r, g, b, a));
            } else if ("_search_select_color" == tagName) {
                st::StyleDefine::instance().setSearchSelectColor(QColor(r, g, b, a));
            } else if ("_image_select_border_color" == tagName) {
                st::StyleDefine::instance().setImageSelectBorderColor(QColor(r, g, b, a));
            } else if ("_nav_top_color" == tagName) {
                st::StyleDefine::instance().setNavTopColor(QColor(r, g, b, a));
            } else if ("_nav_tip_color" == tagName) {
                st::StyleDefine::instance().setNavTipColor(QColor(r, g, b, a));
            } else if ("_nav_name_font_color" == tagName) {
                st::StyleDefine::instance().setNavNameFontColor(QColor(r, g, b, a));
            } else if ("_nav_content_font_color" == tagName) {
                st::StyleDefine::instance().setNavContentFontColor(QColor(r, g, b, a));
            } else if ("_nav_time_font_color" == tagName) {
                st::StyleDefine::instance().setNavTimeFontColor(QColor(r, g, b, a));
            } else if ("_nav_name_font_select_color" == tagName) {
                st::StyleDefine::instance().setNavNameSelectFontColor(QColor(r, g, b, a));
            } else if ("_nav_content_font_select_color" == tagName) {
                st::StyleDefine::instance().setNavContentSelectFontColor(QColor(r, g, b, a));
            } else if ("_nav_time_font_select_color" == tagName) {
                st::StyleDefine::instance().setNavTimeSelectFontColor(QColor(r, g, b, a));
            } else if ("_nav_at_font_color" == tagName) {
                st::StyleDefine::instance().setNavAtFontColor(QColor(r, g, b, a));
            } else if ("_nav_unread_font_color" == tagName) {
                st::StyleDefine::instance().setNavUnReadFontColor(QColor(r, g, b, a));
            } else if ("_chat_group_normal_color" == tagName) {
                st::StyleDefine::instance().setChatGroupNormalColor(QColor(r, g, b, a));
            } else if ("_chat_group_font_color" == tagName) {
                st::StyleDefine::instance().setChatGroupFontColor(QColor(r, g, b, a));
            } else if ("_adr_normal_color" == tagName) {
                st::StyleDefine::instance().setAdrNormalColor(QColor(r, g, b, a));
            } else if ("_adr_select_color" == tagName) {
                st::StyleDefine::instance().setAdrSelectColor(QColor(r, g, b, a));
            } else if ("_adr_name_font_color" == tagName) {
                st::StyleDefine::instance().setAdrNameFontColor(QColor(r, g, b, a));
            } else if ("_drop_normal_color" == tagName) {
                st::StyleDefine::instance().setDropNormalColor(QColor(r, g, b, a));
            } else if ("_drop_select_color" == tagName) {
                st::StyleDefine::instance().setDropSelectColor(QColor(r, g, b, a));
            } else if ("_drop_normal_font_color" == tagName) {
                st::StyleDefine::instance().setDropNormalFontColor(QColor(r, g, b, a));
            } else if ("_drop_select_font_color" == tagName) {
                st::StyleDefine::instance().setDropSelectFontColor(QColor(r, g, b, a));
            } else if ("_link_url_color" == tagName) {
                st::StyleDefine::instance().setLinkUrl(QColor(r, g, b, a));
            } else if ("_local_search_time_font_color" == tagName) {
                st::StyleDefine::instance().setLocalSearchTimeFontColor(QColor(r, g, b, a));
            } else if ("_at_block_font_color" == tagName) {
                st::StyleDefine::instance().setAtBlockFontColor(QColor(r, g, b, a));
            } else if ("_emo_select_icon_color" == tagName) {
                st::StyleDefine::instance().setEmoSelectIconColor(QColor(r, g, b, a));
            } else if ("_group_card_group_member_normal_color" == tagName) {
                st::StyleDefine::instance().setGroupCardGroupMemberNormalColor(QColor(r, g, b,
                        a));
            } else if ("_group_manager_normal_color" == tagName) {
                st::StyleDefine::instance().setGroupManagerNormalColor(QColor(r, g, b, a));
            } else if ("_group_manager_normal_font_color" == tagName) {
                st::StyleDefine::instance().setGroupManagerNormalFontColor(QColor(r, g, b, a));
            } else if ("_title_search_normal_color" == tagName) {
                st::StyleDefine::instance().setTitleSearchNormalColor(QColor(r, g, b, a));
            } else if ("_title_search_select_color" == tagName) {
                st::StyleDefine::instance().setTitleSearchSelectColor(QColor(r, g, b, a));
            } else if ("_quote_block_background_color" == tagName) {
                st::StyleDefine::instance().setQuoteBlockBackgroundColor(QColor(r, g, b, a));
            } else if ("_quote_block_tip_color" == tagName) {
                st::StyleDefine::instance().setQuoteBlockTipColor(QColor(r, g, b, a));
            } else if ("_quote_block_font_color" == tagName) {
                st::StyleDefine::instance().setQuoteBlockFontColor(QColor(r, g, b, a));
            } else if ("_file_process_bar_background" == tagName) {
                st::StyleDefine::instance().setFileProcessBarBackground(QColor(r, g, b, a));
            } else if ("_file_process_bar_line" == tagName) {
                st::StyleDefine::instance().setFileProcessBarLine(QColor(r, g, b, a));
            } else if ("_head_photo_mask_color" == tagName) {
                st::StyleDefine::instance().setHeadPhotoMaskColor(QColor(r, g, b, a));
            } else if ("_hot_line_tip_item_color" == tagName) {
                st::StyleDefine::instance().setHotLineTipItemColor(QColor(r, g, b, a));
            } else if ("_hot_line_tip_item_font_color" == tagName) {
                st::StyleDefine::instance().setHotLineTipItemFontColor(QColor(r, g, b, a));
            }
        }
    }

    delete config;
}
