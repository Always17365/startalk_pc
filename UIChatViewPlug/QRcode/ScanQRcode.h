//
// Created by cc on 2019-04-16.
//

#ifndef STALK_V2_SCANQRCODE_H
#define STALK_V2_SCANQRCODE_H

#include <QFrame>
#include "qzxing/QZXing.h"

/**
* @description: ScanQRcode
* @author: cc
* @create: 2019-04-16 20:23
**/
class QRcode;
class ScanMainFrm;
class QLabel;
class ScanQRcode : public QFrame
{
    Q_OBJECT
public:
    explicit ScanQRcode(QRcode *parent);

public:
    static void qzxingDecodeImage(const QPixmap &pix);

protected:
    bool event(QEvent *e) override;

private:
    void onScan();
    void onScanSuccess(const QString &);
    void scanPixmap(const QPixmap &pix);
    static void showScanResult(const QString &res);

Q_SIGNALS:
    void sgScanSuccess(const QString &);

private:
    ScanMainFrm   *_pScanFrm{};
    QTimer        *_pTimer{};

private:
    QRcode        *_pQRcode{};
};


#endif //STALK_V2_SCANQRCODE_H
