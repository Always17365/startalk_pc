#include "Application.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QDateTime>
#include <thread>
#include <sstream>
#include <QEvent>
#include <iostream>
#include <QMouseEvent>
#include <iostream>
#include <QSettings>
#include <QTranslator>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryFile>
#include <QFontDatabase>
#include <curl/curl.h>
#include <QtConcurrent>
#include <QSslSocket>
#include <QThreadPool>
#include "GlobalManager.h"
#include "LogicManager/LogicManager.h"
#include "DataCenter/Platform.h"
#include "Util/Log.h"
#include "MainWindow.h"
#include "CustomUi/QtMessageBox.h"
#include "MessageManager.h"
#include "EventBus/EventBus.h"

#ifdef Q_OS_MAC
    #include "MacApp.h"
#endif

QString Application::_strlogPath;
QString Application::_strqLogPath;

extern bool _sys_run {false};
Application::Application(int argc, char *argv[])
    : QApplication(argc, argv)
{
    _sys_run = true;
    QString strExcutePath = argv[0];
    //
    auto maxThreadCount = QThreadPool::globalInstance()->maxThreadCount();
    QThreadPool::globalInstance()->setMaxThreadCount(qMin(maxThreadCount * 2, 16));
    strExcutePath = QFileInfo(strExcutePath).absolutePath();
    // 启动日志
    initLogSys();
    //
#ifdef _MACOS
    MacApp::initApp();
#endif
    //设置当前路径
#if defined(_WINDOWS) || defined(_MACOS)
    QString curPath = QApplication::applicationDirPath();
    QDir::setCurrent(curPath);
#endif
    curl_global_init(CURL_GLOBAL_ALL);
    // 加载全局单利platform
    DC.setMainThreadId();
    DC.setExecutePath(strExcutePath.toStdString());
    DC.setExecuteName(QApplication::applicationName().toStdString());
    DC.setProcessId(QApplication::applicationPid());
    QString systemStr = QSysInfo::prettyProductName();
    QString productType = QSysInfo::productType();
    QString productVersion = QSysInfo::productVersion();
    DC.setOSInfo(systemStr.toStdString());
    DC.setOSProductType(productType.toStdString());
    DC.setOSVersion(productVersion.toStdString());
    // Ui管理单例
    _pUiManager = GlobalManager::instance();
    int language = AppSetting::instance().getLanguage();

    if (QLocale::AnyLanguage == language) {
        language = QLocale::system().language();
        AppSetting::instance().setLanguage(language);
    }

    switch (language) {
    case QLocale::English: {
        // 加载翻译文件
        static QTranslator wgtQm;
        wgtQm.load(":/QTalk/config/qtalk_en.qm");
        QApplication::installTranslator(&wgtQm);
        break;
    }

    case QLocale::Korean: {
        //
        static QTranslator qtGloble;
        qtGloble.load(":/QTalk/config/qt_ko.qm");
        QApplication::installTranslator(&qtGloble);
        static QTranslator wgtQm;
        wgtQm.load(":/QTalk/config/qtalk_ko.qm");
        QApplication::installTranslator(&wgtQm);
        break;
    }

    case QLocale::Chinese:
    default: {
        // 加载翻译文件
        static QTranslator qtGloble;
        qtGloble.load(":/QTalk/config/qt_zh_CN.qm");
        QApplication::installTranslator(&qtGloble);
        //
        static QTranslator wgtQm;
        wgtQm.load(":/QTalk/config/widgets_zh_CN.qm");
        QApplication::installTranslator(&wgtQm);
        break;
    }
    }

    //
    initTTF();
    // 加载主Qss文件
    _pUiManager->setStylesForApp();
    // 业务管理单例
    _pLogicManager = LogicManager::instance();
    //
    setWindowIcon(QIcon(":/QTalk/image1/StarTalk.png"));
    QApplication::setStyle("fusion");
    _pMainWnd = new MainWindow;
#ifdef _MACOS
    // 窗口调整
    MacApp::AllowMinimizeForFramelessWindow((QWidget *)_pMainWnd);
    // 多开
    connect(_pMainWnd, &MainWindow::sgRunNewInstance, this, []() {
        QStringList params;
        const QString &cmd = QApplication::applicationFilePath();
        QStringList arguments;
        arguments << "AUTO_LOGIN=OFF";
        QProcess::startDetached(cmd, arguments);
    });
#endif
    bool enableAutoLogin = true;
    QString loginMsg;
    //
    {
        int index = 0;
        QMap<QString, QString> params;

        while (index < argc) {
            QString tmpParam(argv[index++]);

            if (tmpParam.contains("=") && tmpParam.count("=") == 1) {
                params.insert(tmpParam.section("=", 0, 0),
                              tmpParam.section("=", 1, 1));
            }
        }

        if (params.contains("AUTO_LOGIN")) {
            enableAutoLogin = params["AUTO_LOGIN"] == "ON";
        }

        if (params.contains("MSG")) {
            loginMsg = params["MSG"];
        }
    }
    _pMainWnd->InitLogin(enableAutoLogin, loginMsg);
    _pMainWnd->initSystemTray();
    //
    connect(this, &QApplication::applicationStateChanged, this,
            &Application::onApplicationStateChange);
    // exec
    exec();
}

Application::~Application()
{
    EventBus::clearHandle();

    if (nullptr != _pMainWnd) {
        delete _pMainWnd;
        _pMainWnd = nullptr;
    }
}

void logMsgOutput(QtMsgType type,
                  const QMessageLogContext &context,
                  const QString &msg)
{
    QString log;
    QString localMsg(msg);
    localMsg.replace("\"", "");
    log.append(
        QDateTime::currentDateTime().toString("[ yyyy-MM-dd hh:mm:ss.zzz ] "));

    switch (type) {
    case QtDebugMsg:
        log.append("[Debug] ");
        break;

    case QtInfoMsg:
        log.append("[Info] ");
        break;

    case QtWarningMsg:
        log.append("[Warning] ");
        break;

    case QtCriticalMsg:
        log.append("[Critical] ");
        break;

    case QtFatalMsg:
        log.append("[Fatal] ");
        break;
    }

    log.append(localMsg);
    log.append("\n");

#ifndef _WINDOWS

    switch (type) {
    case QtInfoMsg:
        std::cout << "\033[1m\033[34m" << localMsg.toStdString() << "\033[0m" <<
                  std::endl;
        break;

    case QtWarningMsg:
        std::cout << "\033[1m\033[33m" << localMsg.toStdString() << "\033[0m" <<
                  std::endl;
        break;

    case QtCriticalMsg:
        std::cout << "\033[1m\033[31m" << localMsg.toStdString() << "\033[0m" <<
                  std::endl;
        break;

    case QtFatalMsg:
        std::cout << "\033[1m\033[35m" << localMsg.toStdString() << "\033[0m" <<
                  std::endl;
        break;

    default:
    case QtDebugMsg:
        std::cout << "\033[1m\033[36m" << localMsg.toStdString() << "\033[0m" <<
                  std::endl;
        break;
    }

#else
    std::cout << log.toStdString() << std::endl;
#endif
    QFile file(Application::_strqLogPath);

    if (!file.isOpen() && !file.open(QIODevice::ReadWrite | QIODevice::Append)) {
        QString erinfo = file.errorString();
    } else {
        static st::util::spin_mutex sm;
        std::lock_guard<st::util::spin_mutex> lock(sm);
        file.write(log.toUtf8());
        file.flush();
    }
}

/**
  * @函数名   initLogSys
  * @功能描述 启动日志系统
  * @参数
  * @author   cc
  * @date     2018/10/11
  */
#include <functional>
void Application::initLogSys()
{
    QDateTime curDateTime = QDateTime::currentDateTime();
    QString appdata = QStandardPaths::writableLocation(
                          QStandardPaths::AppDataLocation);
    _strlogPath = QString("%1/logs/%2/")
                  .arg(appdata)
                  .arg(curDateTime.toString("yyyy-MM-dd"));
    _strqLogPath = QString("%1/%2_qt.log").arg(_strlogPath).arg(
                       curDateTime.toString("yyyy-MM-dd-hh-mm-dd"));
    QDir logDir(_strlogPath);

    if (!logDir.exists()) {
        logDir.mkpath(_strlogPath);
    }

    qInstallMessageHandler(logMsgOutput);
    //
    info_log("system started . client version:{0}", DC.getClientVersion());
    qInfo() << "system started . client version: "
            << DC.getClientNumVerison()
            << DC.get_build_date_time().data()
            << "max thread count:" << QThreadPool::globalInstance()->maxThreadCount();
}

/**
  * @函数名   notify
  * @功能描述
  * @参数
  * @author   cc
  * @date     2018/10/11
  */
bool Application::notify(QObject *receiver, QEvent *e)
{
    try {
        switch (e->type()) {
        case QEvent::MouseButtonPress: {
            auto *mouseEvent = dynamic_cast<QMouseEvent *>(e);

            if (_pUiManager) {
                emit _pUiManager->sgMousePressGlobalPos(mouseEvent->globalPos());
            }

            if (_pMainWnd) {
                emit _pMainWnd->sgResetOperator();
            }

            break;
        }

        default:
            break;
        }

        return QApplication::notify(receiver, e);
    } catch (const std::bad_alloc &) {
        qWarning() << "Out of memory";
        QApplication::exit(-1);
    } catch (const std::exception &e) {
        return false;
    }
}

//
bool Application::event(QEvent *e)
{
#ifdef Q_OS_MAC

    if (e->type() == QEvent::FileOpen) {
        auto *event = dynamic_cast<QFileOpenEvent *>(e);

        if (!event->url().isEmpty()) {
            auto url = event->url();
            QUrlQuery query(url.query());
            auto id = query.queryItemValue("id");
            auto name = query.queryItemValue("name");
            qInfo() << "recv url scheme" << url << id;
            e->ignore();
        }
    }

#endif
    return QApplication::event(e);
}

//
void Application::onApplicationStateChange(Qt::ApplicationState state)
{
    switch (state) {
    case Qt::ApplicationSuspended: {
        qInfo() << "app will be Suspend";
        break;
    }

    case Qt::ApplicationHidden:
    case Qt::ApplicationInactive: {
        if (nullptr != _pMainWnd) {
            _pMainWnd->onAppDeactivate();
        }

        break;
    }

    case Qt::ApplicationActive: {
        if (nullptr != _pMainWnd) {
            _pMainWnd->onAppActive();
        }

        break;
    }

    default:
        break;
    }
}

void Application::initTTF()
{
    std::string appDataPath = DC.getAppdataRoamingPath();
    QString ttfPath = QString("%1/ttf").arg(appDataPath.data());
    {
        QString tmpTTF = QString("%1/%2").arg(ttfPath, "FZLTHJW.TTF");

        if (!QFile::exists(tmpTTF)) {
            if (!QFile::exists(ttfPath)) {
                QDir dir(appDataPath.data());
                dir.mkpath(ttfPath);
            }

            QTemporaryFile *tmpFile =
                QTemporaryFile::createNativeFile(":/QTalk/ttf/FZLTHJW.TTF");
            tmpFile->copy(tmpTTF);
        }

        QFontDatabase::addApplicationFont(tmpTTF);
    }
}
