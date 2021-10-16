//
// Created by cc on 2019-07-12.
//

#ifndef STALK_V2_GIFMANAGER_H
#define STALK_V2_GIFMANAGER_H

#include <QMap>
#include <QObject>
#include <QMovie>
#include <QString>
#include "Util/Spinlock.h"

/**
* @description: GIFManager
* @author: cc
* @create: 2019-07-12 16:38
**/
class GIFManager{
public:
    GIFManager();

public:
    QMovie* getMovie(const QString& filePath);
    void removeMovie(QMovie *mov);

private:
//    QMap<QString, QMovie*>      _mapMovie;
//    QMap<QMovie*, int>          _movieCount;
    st::util::spin_mutex     sm;
};


#endif //STALK_V2_GIFMANAGER_H
