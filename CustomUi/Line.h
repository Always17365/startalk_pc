//
// Created by cc on 19-1-4.
//

#ifndef STALK_V2_LINE_H
#define STALK_V2_LINE_H

#include <QFrame>

class Line : public QFrame
{
public:
    explicit Line( QWidget *parent = nullptr)
        : Line(Qt::Horizontal, parent)
    {

    }

    explicit Line(Qt::Orientation direction = Qt::Horizontal, QWidget *parent = nullptr)
        : QFrame(parent)
    {
        setObjectName("Line");

        if (direction == Qt::Horizontal) {
            setFixedHeight(1);
            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        } else if (direction == Qt::Vertical) {
            setFixedWidth(1);
            setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        }
    }

};

#endif //STALK_V2_LINE_H
