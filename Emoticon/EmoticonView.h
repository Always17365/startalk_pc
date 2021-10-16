//
// Created by cc on 2019-05-22.
//

#ifndef STALK_V2_EMOTICONVIEW_H
#define STALK_V2_EMOTICONVIEW_H

#include <QFrame>
#include <QMovie>

/**
* @description: EmoticonView
* @author: cc
* @create: 2019-05-22 18:03
**/
class EmoticonView : public QFrame{
public:
    explicit EmoticonView(QWidget* parent = nullptr);
    ~EmoticonView() override;

public:
    void setImagePath(const QString& imgPath);
    void releaseView();

protected:
    void paintEvent(QPaintEvent* e) override;


private:
    QString _imagePath;
    QMovie* _mov;
    int     _width;
};


#endif //STALK_V2_EMOTICONVIEW_H
