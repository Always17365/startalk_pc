//
// Created by cc on 18-11-9.
//

#ifndef STALK_V2_TIPMESSAGEITEM_H
#define STALK_V2_TIPMESSAGEITEM_H


#include "MessageItemBase.h"
#include <QLabel>
#include "Util/nJson/nJson.h"

class QLabel;
class TipMessageItem : public QFrame
{
    Q_OBJECT
public:
    explicit TipMessageItem(QWidget *parent = nullptr);
    ~TipMessageItem() override;

public:
    void setText(const QString &text);
    QString getText();

protected:
    void resizeEvent(QResizeEvent *e) override;

protected slots:
    void openURL(QString url);

private:
    QString _strText;
    QLabel *_pLabel;
};


#endif //STALK_V2_TIPMESSAGEITEM_H
