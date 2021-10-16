#include "StyleManager.h"
#include <QFile>
#include <QApplication>
#include <set>
#include <QDebug>
#include "GlobalManager.h"
#include "interface/view/UIPluginInterface.h"

//
void adjustFontSize(QString& qss)
{
    auto fontLevel = AppSetting::instance().getFontLevel();
    if(AppSetting::FONT_LEVEL_NORMAL == fontLevel)
        return;

    QRegExp exp("font-size[\\ ]*:[\\ ]*(\\d+)px[;]{0,1}");
    int pos = 0;
    while ((pos = exp.indexIn(qss, pos)) != -1) {
        auto item = exp.cap(0); // 符合条件的整个字符串
        auto size = exp.cap(1);

        if(AppSetting::FONT_LEVEL_BIG == fontLevel)
            item.replace(size, QString::number(size.toInt() + 2));
        else if(AppSetting::FONT_LEVEL_SMALL == fontLevel)
            item.replace(size, QString::number(size.toInt() - 1));
        else
            return;

        qss.replace(pos, item.size(), item);
        pos += item.size();
    }
}

/**
  * @函数名
  * @功能描述
  * @参数
  * @date 2018.9.17
  */
void StyleManager::setStyleSheets(int theme, const std::string& font)
{
    // 设置app qss
    setStylesForApp(theme, font);
    //
    std::shared_ptr<QMap<QString, QObject *> > plugins = GlobalManager::instance()->getAllPluginInstanceQt();
    for (const QString& pluginName : plugins->keys()) {
        QObject * plugin = plugins->value(pluginName);
        if (plugin)
        {
            auto *plug = dynamic_cast<UIPluginInterface *>(plugin);
            if (plug)
            {
                QString styleSheetPath = QString(":/style/style%1/%2.qss").arg(theme).arg(pluginName);
                if (QFile::exists(styleSheetPath))
                {
                    QFile file(styleSheetPath);
                    if (file.open(QFile::ReadOnly))
                    {
                        QString qss = QString::fromUtf8(file.readAll()); //以utf-8形式读取文件
                        adjustFontSize(qss);
                        plug->setStyleSheet(qss.toUtf8());
                    }
                }
            }
        }
    }
}

/**
  * @函数名
  * @功能描述
  * @参数
  * @date 2018.9.17
  */
void StyleManager::setStylesForApp(int theme, const std::string& font)
{
    QString qss;
    //
    if(font.empty())
    {
        QFont tmpFont;
#ifdef _WDINWOS
        tmpFont.fromString(tr("方正兰亭黑简体"));
        qss.append(QString("* {font-family: \"方正兰亭黑简体\";}"));
#else
        tmpFont.fromString("FZLanTingHeiS-R-GB");
        qss.append(QString("* {font-family: \"FZLanTingHeiS-R-GB\";}"));
#endif
        qApp->setFont(tmpFont);
    }
    else
    {
        QFont tmpFont;
        tmpFont.fromString(font.data());
        qApp->setFont(tmpFont);
        //
        qss.append(QString("* {font-family: \"%1\";}").arg(font.data()));
    }
    QString apppath = QString(":/style/style%1/app.qss").arg(theme);
    if (QFile::exists(apppath))
    {
        QFile fileapp(apppath);
        if (fileapp.open(QFile::ReadOnly))
        {
            qss.append(fileapp.readAll().data());
            adjustFontSize(qss);
            qApp->setStyleSheet(qss.toUtf8());
        }
    }
}

/**
  * @函数名
  * @功能描述
  * @参数
  * @date 2018.9.17
  */
void StyleManager::setStyleSheetForPlugin(const QString& plgName, int theme)
{
    QString styleSheetPath = QString(":/style/style%1/%2.qss").arg(theme).arg(plgName);
//    QJsonObject obj = Get("styles").toObject();
//    QString stylePath = styleSheetPath + obj.value(plgName).toString();
    if (QFile::exists(styleSheetPath))
    {
        QFile file(styleSheetPath);
        QObject* plugin = GlobalManager::instance()->getPluginInstanceQt(plgName);
        if (plugin)
        {
            auto *plug = dynamic_cast<UIPluginInterface *>(plugin);
            if (plug)
            {
                if (file.open(QFile::ReadOnly))
                {
                    QString qss = QString::fromUtf8(file.readAll()); //以utf-8形式读取文件
                    plug->setStyleSheet(qss.toUtf8());
                }
            }
        }
    }

}
