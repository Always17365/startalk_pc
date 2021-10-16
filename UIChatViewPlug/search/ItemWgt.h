//
// Created by cc on 2019-06-26.
//

#ifndef STALK_V2_ITEMWGT_H
#define STALK_V2_ITEMWGT_H

#include <QFrame>
#include <QMenu>

struct StFileData {
    int     status;
    QString fileName;
    QString fileSize;
    QString fileUrl;
    QString filePath;
    QString fileMd5;
    QString iconPath;
};

struct StSearchData {
    QString searchKey;
};

struct StData {
    int     type;
    QString messageId;
    qint64  time;
    QString userName;
    int     direction;
    QString content;

    StFileData fileData;
    StSearchData searchData;
};

enum {

    EM_FILE_STATUS_INVALID,
    EM_FILE_STATUS_UN_DOWNLOAD,
    EM_FILE_STATUS_DOWNLOADING,
    EM_FILE_STATUS_DOWNLOADED,
};

class NameTitleLabel : public QFrame {

public:
    explicit NameTitleLabel(int dir, QString name, QString time, QWidget* parent = nullptr);
    ~NameTitleLabel() override = default;

protected:
    void paintEvent(QPaintEvent* e) override ;

private:
    int _dir;
    QString _name;
    QString _time;
};

#endif //STALK_V2_ITEMWGT_H
