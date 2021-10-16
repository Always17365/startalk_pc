//
// Created by cc on 2019-04-16.
//

#ifndef STALK_V2_MAKEQRCODE_H
#define STALK_V2_MAKEQRCODE_H

#include <QFrame>
#include <QMenu>
#include <QAction>

/**
* @description: makeQRcode
* @author: cc
* @create: 2019-04-16 20:30
**/
class QRcode;
class QLabel;
class QTextEdit;
class MakeQRcode : public QFrame {
	Q_OBJECT
public:
    explicit MakeQRcode(QRcode* parent = nullptr);
    ~MakeQRcode() override;

public:
    void resetWnd();

protected:
    bool event(QEvent* e) override;
    bool eventFilter(QObject* o, QEvent* e) override;

private:
    void onCopyAct(bool);
    void onSaveAct(bool);

private:
    QRcode* _pQRcode;
    QLabel* _pQRCodeLabel;
    QTextEdit* _pInputEdit;

private:
    QMenu* _pMenu;

    QPixmap _pixmap;
};


#endif //STALK_V2_MAKEQRCODE_H
