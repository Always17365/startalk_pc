//
// Created by cc on 2019-02-28.
//

#ifndef STALK_V2_CodeItem_H
#define STALK_V2_CodeItem_H

#include "MessageItemBase.h"

#include <QLabel>


class CodeItem : public MessageItemBase
{

    Q_OBJECT
public:
    explicit CodeItem(const StNetMessageResult &msgInfo, QWidget *parent = nullptr);

public:
    QSize itemWdtSize() override;

protected:
    void mousePressEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *o, QEvent *e) override;

private:
    void init();
    void initLayout();
    void initSendLayout();
    void initReceiveLayout();
    void initContentLayout();

private:
    QLabel *_iconLab {nullptr};
    QLabel *_titleLab{nullptr};
    QLabel *_contentLab{nullptr};

    QSize _headPixSize;
    QMargins _mainMargin;
    QMargins _leftMargin;
    QMargins _rightMargin;
    QSize _contentSize;
    int _mainSpacing {0};

    int _leftSpacing  {0};
    int _rightSpacing {0};
    int _nameLabHeight{0};

public:
    QString _codeStyle;
    QString _codeLanguage;
    QString _code;

};


#endif //STALK_V2_CodeItem_H
