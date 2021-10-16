//
// Created by cc on 2019-07-15.
//
#include "ChatUtil.h"
#include "Util/ui/qimage/qimage.h"
#include "ChatViewMainPanel.h"
#include <QFile>
#include <QFileInfo>
#include <QtConcurrent>

extern ChatViewMainPanel *g_pMainPanel;
namespace st
{
    namespace Image {

        void scaImageSize(qreal &width, qreal &height) {

            if(width == 0 || height == 0)
            {
                width = height = 200;
            }

            double factor = 1.0;
            const double maxsize = 200;
            double factor1 = maxsize / width;
            double factor2 = maxsize / height;
            if (factor1 < 1.0 || factor2 < 1.0) {
                factor = qMin(factor1, factor2);
            }

            width = factor * width;
            height = factor * height;

            if(factor == 1 && (width < 50 && height < 50))
            {
                width = height = 50;
            }
        }

        void scaImageSizeByPath(const QString &imagePath, qreal &width, qreal &height)
        {
            QPixmap pix = qimage::loadImage(imagePath, false);
            width = pix.width();
            height = pix.height();

            scaImageSize(width, height);
        }

    }

    namespace File {

        QString getRealFilePath(const std::string &msgId, const std::string &md5) {
            QString fullFileName = QString::fromStdString(g_pMainPanel->getFileMsgLocalPath(msgId));
            if (fullFileName.isEmpty() || !QFile::exists(fullFileName)) {
                QString linkFile = g_pMainPanel->getFileLink(md5.data());
#ifdef _WINDOWS
                linkFile.append(".lnk");
#endif
                QFileInfo linkFileInfo(linkFile);
                if (linkFileInfo.exists() && !linkFileInfo.canonicalFilePath().isEmpty())
#ifdef _WINDOWS
                    return linkFileInfo.symLinkTarget();
#else
                    return linkFileInfo.canonicalFilePath();
#endif // _WINDOWS
                else
                    return QString();
            } else {
                return fullFileName;
            }
        }

        void openFileFolder(const QString& filePath)
        {
            QFileInfo info(filePath);
            QtConcurrent::run([info](){

#if defined(_WINDOWS)
                QStringList params;
                QString path = info.absoluteFilePath();
                if (!QFileInfo(path).isDir()) {
                    params << QLatin1String("/select,");
                    params << QDir::toNativeSeparators(path);
                    QProcess::startDetached(QLatin1String("explorer.exe"), params);
                }
#elif defined(_MACOS)
                QStringList scriptArgs;
                QDesktopServices::openUrl(QUrl::fromLocalFile(info.absolutePath()));
                scriptArgs << QLatin1String("-e")
                           << QString::fromLatin1(R"(tell application "Finder" to reveal POSIX file "%1")")
                                   .arg(info.absoluteFilePath());
                QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
#else
                QDesktopServices::openUrl(QUrl::fromLocalFile(info.absolutePath()));
#endif
            });
        }
    }

    //
    QString getIconByFileName (const QString& fileName)
    {
        QFileInfo iconInfo(fileName);
        QString iconPath;
        QString suffix = iconInfo.suffix().toLower();
        if(suffix == "mp3")
            iconPath = ":/QTalk/image1/file_type/audio.png";
        else if(suffix == "mp4")
            iconPath = ":/QTalk/image1/file_type/video.png";
        else if(suffix == "txt" || suffix == "json")
            iconPath = ":/QTalk/image1/file_type/text.png";
        else if(suffix == "pdf")
            iconPath = ":/QTalk/image1/file_type/pdf.png";
        else if(suffix == "ppt" || suffix == "pptx")
            iconPath = ":/QTalk/image1/file_type/ppt.png";
        else if(suffix == "doc" || suffix == "docx")
            iconPath = ":/QTalk/image1/file_type/word.png";
        else if(suffix == "xls" || suffix == "xlsx")
            iconPath = ":/QTalk/image1/file_type/excel.png";
        else if(suffix == "rar" || suffix == "zip" || suffix == "7z")
            iconPath = ":/QTalk/image1/file_type/zip.png";
        else
            iconPath = ":/QTalk/image1/file_type/unknown.png";

        return iconPath;
    }

    QLabel* makeLoadingLabel(bool scale, QSize size, QWidget* parent)
    {
        auto* label = new QLabel(parent);
        auto* mov = new QMovie(label);
        mov->setFileName(":/QTalk/image1/loading.gif");
        if (scale)
        {
            mov->setScaledSize(size);
            label->setFixedHeight(size.height());
        }

        label->setMovie(mov);
        label->setAlignment(Qt::AlignCenter);
        return label;
    }
}
