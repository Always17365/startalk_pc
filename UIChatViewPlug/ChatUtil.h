//
// Created by cc on 2019-07-15.
//
#include <QObject>
#include <QPixmap>
#include <QLabel>
#include <QMovie>

namespace st {

    namespace Image {
        void scaImageSize(qreal &width, qreal &height);
        void scaImageSizeByPath(const QString &imagePath, qreal &width, qreal &height);
    }

    namespace File {
        QString getRealFilePath(const std::string& msgId, const std::string& md5);
        void openFileFolder(const QString& filePath);
    }

    //
    QString getIconByFileName (const QString& fileName);

    QLabel* makeLoadingLabel(bool scale = false, QSize size = {0, 0}, QWidget* parent = nullptr);
}