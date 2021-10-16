//
// Created by cc on 2020/1/13.
//

#ifndef STALK_V2_IMAGELABEL_H
#define STALK_V2_IMAGELABEL_H

#include "customui_global.h"
#include <QFrame>

class CUSTOMUISHARED_EXPORT ImageLabel : public QFrame{
    Q_OBJECT
public:
    explicit ImageLabel(const QString& imagePath, QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* e) override ;

private:
    QString _path;
};


#endif //STALK_V2_IMAGELABEL_H
