//
// Created by cc on 2019-04-16.
//

#include "ScanQRcode.h"
#include <QVBoxLayout>
#include <QTimer>
#include <QDateTime>
#include <QDesktopWidget>
#include <QScreen>
#include <QApplication>
#include <QEvent>
#include <QDesktopServices>

#include "QRcode.h"
#include "ScanMainFrm.h"
#include "TipButton.h"
#include "../ChatViewMainPanel.h"
#include "WebService/WebService.h"
#include "CustomUi/Line.h"
#include "CustomUi/QtMessageBox.h"
#include "Util/ui/uicom.h"
#include "DataCenter/AppSetting.h"

extern ChatViewMainPanel *g_pMainPanel;
ScanQRcode::ScanQRcode(QRcode *parent )
    : QFrame(parent), _pQRcode(parent)
{
    //
    _pScanFrm = new ScanMainFrm(this);
    _pScanFrm->setContentsMargins(60, 50, 60, 50);

    auto *mainLay = new QVBoxLayout(this);
    mainLay->setMargin(0);
    mainLay->addWidget(new Line(Qt::Horizontal, this), 1);
    mainLay->addWidget(_pScanFrm, 380);

    _pTimer = new QTimer(this);
    _pTimer->setInterval(3000);
    connect(_pTimer, &QTimer::timeout, this, &ScanQRcode::onScan);
    connect(this, &ScanQRcode::sgScanSuccess, this, &ScanQRcode::onScanSuccess,
            Qt::QueuedConnection);
}

/**
 * 扫描处理
 */
void ScanQRcode::onScan()
{
    QPoint pos = mapToGlobal(_pScanFrm->geometry().topLeft());
#ifdef _LINUX
    QScreen *screen = nullptr;
    auto lstScreens = QApplication::screens();

    for (QScreen *tmps : lstScreens) {
        if (tmps->geometry().contains(pos)) {
            screen = tmps;
            break;
        }
    }

#else
    QScreen *screen = QApplication::screenAt(pos);
#endif
    auto *desktop = QApplication::desktop();
    WId id = desktop->winId();
    QPixmap pix = screen->grabWindow(id,
                                     pos.x() + 50, pos.y() + 50,
                                     _pScanFrm->width() - 100, _pScanFrm->height() - 100);
    scanPixmap(pix);
}

/**
 * event时间 -> timer 处理
 * @param e
 * @return
 */
bool ScanQRcode::event(QEvent *e)
{
    if (e->type() == QEvent::Show) {
        if (!_pTimer->isActive()) {
            _pTimer->start();
        }

        _pScanFrm->start();
    } else if (e->type() == QEvent::Hide) {
        if (_pTimer->isActive()) {
            _pTimer->stop();
        }

        _pScanFrm->stop();
    }

    return QFrame::event(e);
}

void ScanQRcode::onScanSuccess(const QString &ret)
{
    if (_pTimer && _pTimer->isActive()) {
        _pTimer->stop();
    }

    showScanResult(ret);


    _pQRcode->setVisible(false);
}

void ScanQRcode::scanPixmap(const QPixmap &pix)
{
    QPointer<ScanQRcode> pThis(this);
    QT_CONCURRENT_FUNC([pThis, pix]() {
        QImage img = pix.toImage();

        if (!pThis) {
            return;
        }

        int width = pThis->_pScanFrm->width();
        int height = pThis->_pScanFrm->height();
        static QZXing  _qzxing;
        QString ret = _qzxing.decodeImage(img, width, height, true);

        if (!pThis) {
            return;
        }

        if (!ret.isEmpty()) {
            emit pThis->sgScanSuccess(ret);
        }

    });
}

//
void ScanQRcode::qzxingDecodeImage(const QPixmap &pix)
{
    static QZXing  _qzxing;
    auto res = _qzxing.decodeImage(pix.toImage(),
                                   pix.width(), pix.height(), true);

    if (res.isEmpty()) {
        emit g_pMainPanel->sgShowInfoMessageBox(tr("无法识别二维码"));
    } else {
        showScanResult(res);
    }
}

//
void ScanQRcode::showScanResult(const QString &res)
{

    QUrl url = QUrl::fromUserInput(res);

    if (res.startsWith("qtalk://group?id=")) {
        QString groupId = res.section("id=", 1, 1);
        std::string selfId = g_pMainPanel->getSelfUserId();
        //
        int retBtn = QtMessageBox::question(g_pMainPanel, tr("提醒"),
                                            QString(tr("是否加入群聊%1")).arg(groupId.section("@", 0, 0)));

        if (retBtn != QtMessageBox::EM_BUTTON_YES) {
            return;
        }

        //
        std::vector<std::string> members;
        members.push_back(selfId);
        ChatMsgManager::addGroupMember(members, groupId.toStdString());
    } else {
        if (res.startsWith("http") ||
            res.startsWith("www")) {
            int ret_btn = QtMessageBox::question(g_pMainPanel, tr("提醒"),
                                                 QString(tr("是否使用浏览器打开 %1 ?")).arg(res));

            if (QtMessageBox::EM_BUTTON_YES == ret_btn) {
                QDesktopServices::openUrl(url);
            }
        } else {
            QtMessageBox::information(g_pMainPanel, tr("扫一扫结果"), res);
        }
    }
}
