//
// Created by cc on 2019/11/13.
//

#ifndef STALK_V2_NETIMAGELABEL_H
#define STALK_V2_NETIMAGELABEL_H

#include <QFrame>

/**
* @description: NetImageLabel
* @author: cc
* @create: 2019-11-13 15:13
**/
class NetImageLabel : public QFrame {
    Q_OBJECT
public:
    explicit NetImageLabel(QString  link, QWidget* parent = nullptr);

public:
    void setLocalImage(const QString& local);
    void showVideoMask();

protected:
    void paintEvent(QPaintEvent* event) override ;
    bool event(QEvent* e) override ;

Q_SIGNALS:
    void sgDownloadSuccess();
    void clicked();

protected:
    QString _image_link{};
    QString _local_path;

    bool _showVideoMask{};
};


#endif //STALK_V2_NETIMAGELABEL_H
