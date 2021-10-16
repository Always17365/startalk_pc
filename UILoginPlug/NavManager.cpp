//
// Created by cc on 2018-12-25.
//

#include "NavManager.h"
#include <QFile>
#include <QDir>
#include <QUrl>
#include <QUrlQuery>
#include "CustomUi/QtMessageBox.h"
#include "LoginPanel.h"
#include "DataCenter/Platform.h"
#include "Util/ui/qconfig/qconfig.h"

#define DEM_NAV_KEYS "navKeys"
#define DEM_DEFAULT_KEY "defaultKey"
#define DEM_CONFIG_VERSION "version"

#define DEM_DEFAULT_NAV ""

#define DEM_ROOT_TAG "NavConfig"

NavManager::NavManager(LoginPanel *loginPanel)
    : UShadowDialog(loginPanel, true)
    , _pLoginPanel(loginPanel)
{
    initConfig();
    initUi();
}

void NavManager::initConfig()
{
    std::string configDirPath = DC.getConfigPath();
    std::string newConfig = configDirPath + "/NavConf.data";
    {
        auto *navConfig = new st::StConfig;

        if (!QFile::exists(configDirPath.data())) {
            QDir dir;
            dir.mkpath(configDirPath.data());
        }

        //
        if (QFile::exists(newConfig.data())) {
            st::qConfig::loadConfig(newConfig.data(), false, navConfig);

            if (navConfig->tagName == DEM_ROOT_TAG &&
                navConfig->hasAttribute(DEM_DEFAULT_KEY) &&
                !navConfig->children.empty()) {
                _defaultKey = navConfig->attribute(DEM_DEFAULT_KEY);

                for (const auto &item : navConfig->children) {
                    StNav stNav;
                    stNav.name = item->attribute("name");
                    stNav.domain = item->attribute("domain");
                    stNav.debug = item->attribute("debug") == "true";
                    stNav.url = item->attribute("url");
                    _mapNav[stNav.name] = stNav;
                }

                // delete
                delete navConfig;
                return;
            } else {
                qWarning() << "invalid nav config, --> delete it";
                QFile::remove(configDirPath.data());
            }
        }

        delete navConfig;
    }
}

void NavManager::initUi()
{
    setFixedSize(630, 500);
    // top
    auto *topFrm = new QFrame(this);
    topFrm->setObjectName("NavManager_TopFrm");
    topFrm->setFixedHeight(50);
    auto *topLay = new QHBoxLayout(topFrm);
    auto *titleLabel = new QLabel(tr("配置导航"));
    titleLabel->setObjectName("NavManager_TitleLabel");
    _pCloseBtn = new QPushButton();
    titleLabel->setAlignment(Qt::AlignCenter);
    topLay->addWidget(titleLabel);
    topLay->addWidget(_pCloseBtn);
#ifdef _MACOS
    _pCloseBtn->setFixedSize(12, 12);
    _pCloseBtn->setObjectName("gmCloseBtn");
    topLay->addWidget(_pCloseBtn);
    topLay->addWidget(titleLabel);
    topLay->setContentsMargins(15, 6, 0, 0);
#else
    _pCloseBtn->setFixedSize(30, 30);
    _pCloseBtn->setObjectName("gwCloseBtn");
    topLay->setMargin(0);
    topLay->addWidget(titleLabel);
    topLay->addWidget(_pCloseBtn);
#endif
    //
    this->setMoverAble(true, topFrm);
    // main left
    auto *mainFrame = new QFrame(this);
    mainFrame->setObjectName("NavManager_MainFrm");
    auto *mainLay = new QHBoxLayout(mainFrame);
    mainLay->setMargin(0);
    _pNavView = new NavView(_mapNav, _defaultKey, this);
    mainLay->addWidget(_pNavView);

    for (const StNav &nav  : _mapNav.values()) {
        emit _pNavView->addItemSignal(nav);
    }

    // main
    auto *layout = new QVBoxLayout(_pCenternWgt);
    layout->addWidget(topFrm);
    layout->addWidget(mainFrame);
    layout->setMargin(0);
    layout->setSpacing(0);
    layout->setStretch(1, 1);
    // 关闭按钮
    connect(_pCloseBtn, &QPushButton::clicked, this, [this](bool) {
        this->setVisible(false);
    });
    connect(_pNavView, &NavView::saveConfSignal, this, &NavManager::onSaveConf);
    connect(_pNavView, &NavView::addNavSinal, this, &NavManager::onAddNav);
    connect(_pNavView, &NavView::sgNavChanged, this, &NavManager::onNavChanged,
            Qt::QueuedConnection);
    connect(_pNavView, &NavView::sgClose, this, [this]() {
        this->setVisible(false);
    });
    //
    _pNavView->onItemClicked(_defaultKey);
}

/**
 * 写配置
 */
void NavManager::saveConfig()
{
    std::string newConfig = DC.getConfigPath() + "/NavConf.data";
    auto *navConfig = new st::StConfig(DEM_ROOT_TAG);

    for (const StNav &nav : _mapNav.values()) {
        if (nav.url.isEmpty() || nav.name.isEmpty() || nav.domain.isEmpty()) {
            continue;
        }

        auto *item = new st::StConfig("item");
        item->setAttribute("name", nav.name);
        item->setAttribute("domain", nav.domain);
        item->setAttribute("url", nav.url);
        QString debug = (nav.debug ? "true" : "false");
        item->setAttribute("debug", debug);
        navConfig->addChild(item);
    }

    if (!navConfig->children.empty()) {
        navConfig->setAttribute(DEM_DEFAULT_KEY,
                                _defaultKey.isEmpty() ?  navConfig->children[0]->attribute("name") :
                                _defaultKey);
        navConfig->setAttribute(DEM_CONFIG_VERSION, 1);
        //
        st::qConfig::saveConfig(newConfig.data(), false, navConfig);
    }

    delete navConfig;
}

/**
 * 获取导航地址
 * @return
 */
QString NavManager::getDefaultNavUrl()
{
    if (_mapNav.contains(_defaultKey)) {
        StNav nav = _mapNav.value(_defaultKey);
        return nav.url;
    }

    return QString();
}

/**
 *
 */
void NavManager::onSaveConf()
{
    saveConfig();
}

/**
 *
 * @return
 */
QString NavManager::getNavName()
{
    bool debug = _mapNav[_defaultKey].debug;
    QString ret = _mapNav[_defaultKey].domain;

    if (debug) {
        ret += "_debug";
    }

    return ret;
}

/**
 *
 * @param name
 * @param navAddr
 * @param isDebug
 */
void NavManager::onAddNav(const QString &name, const QString &navAddr,
                          const bool &isDebug)
{
    const QString &addr = navAddr;
    QString domain = _pLoginPanel->getDomainByNav(addr);

    if (domain.isEmpty()) {
        QtMessageBox::warning(this, QObject::tr("警告"), tr("无效的导航地址"));
    } else {
        StNav nav;
        nav.name = name;
        nav.url = navAddr;
        nav.domain = domain;
        nav.debug = isDebug;
        _mapNav[name] = nav;
        //
        emit _pNavView->addItemSignal(nav);
        //
        saveConfig();
    }
}

QString NavManager::getDefaultDomain()
{
    if (!_defaultKey.isEmpty() && _mapNav.contains(_defaultKey)) {
        QString tmp = _mapNav[_defaultKey].domain;

        if (!tmp.isEmpty() && _mapNav[_defaultKey].debug) {
            tmp += "_debug";
        }

        return tmp;
    }

    return QString();
}

// 切换导航
void NavManager::onNavChanged()
{
    //
    if (_pLoginPanel) {
        _pLoginPanel->loadConf();
    }
}
