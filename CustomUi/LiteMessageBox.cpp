//
// Created by cc on 2019-01-27.
//

#include "LiteMessageBox.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QTimer>
#include <QDebug>
#include "Util/ui/uicom.h"
#include "Util/ui/qimage/qimage.h"


LiteMessageBox::LiteMessageBox(int type, const QString &message, QWidget *base)
{
    Q_INIT_RESOURCE(ushadowdialog);
    Qt::WindowFlags flags = Qt::Dialog | Qt::WindowContextHelpButtonHint | Qt::FramelessWindowHint
                            | Qt::WindowFullscreenButtonHint | Qt::WindowCloseButtonHint | Qt::WindowTitleHint;
    this->setWindowFlags(flags);
    this->setAttribute(Qt::WA_ShowWithoutActivating, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    //
    auto *iconLab = new QLabel(this);
    auto *textLab = new QLabel(this);
    textLab->setObjectName("LiteMessageBoxText");
    //
    //iconLab->setFixedSize(14, 14);
    auto *mainFrm = new QFrame(this);
    mainFrm->setObjectName("LiteMessageBoxMainFrm");

    switch (type) {
    case EM_TYPE_SUCCESS:

        mainFrm->setObjectName("LiteMessageBoxSuccess");
        iconLab->setPixmap(st::qimage::loadImage(":/success", true, true, 15, 15));
        break;

    case EM_TYPE_FAILED:
        mainFrm->setObjectName("LiteMessageBoxFailed");
        iconLab->setPixmap(st::qimage::loadImage(":/error", true, true, 15, 15));
        break;

    case EM_TYPE_INVALID:
    default:
        break;
    }

    textLab->setText(message);
    //

    auto *mainLay = new QHBoxLayout(mainFrm);
    mainLay->setMargin(10);
    mainLay->setSpacing(8);
    mainLay->addWidget(iconLab);
    mainLay->addWidget(textLab);
    //
    auto *lay = new QHBoxLayout(this);
    lay->setMargin(0);
    lay->addWidget(mainFrm);

    //
    QWidget *wgt = nullptr;

    if (nullptr != base) {
        wgt = base;
    } else {
        wgt = UICom::getInstance()->getAcltiveMainWnd();
    }

    //setParent(wgt);
    if (nullptr != wgt) {
        adjustSize();
        QPoint pos = wgt->mapToGlobal(QPoint((wgt->width() - this->width()) / 2, 50));
        this->move(pos);
    }

}

LiteMessageBox::~LiteMessageBox()
{
    qInfo() << "LiteMessageBox" << "deleted";
}

void LiteMessageBox::success(const QString &message, int duration, QWidget *base)
{
    static std::set<LiteMessageBox *> _arBox;

    for (auto *box : _arBox) {
        box->setVisible(false);
    }

    auto *box = new LiteMessageBox(EM_TYPE_SUCCESS, message, base);
    _arBox.insert(box);
    box->setAttribute(Qt::WA_QuitOnClose, false);
    box->setVisible(true);

    if (duration <= 1000) {
        duration = 1000;
    }

    QTimer::singleShot(duration, [box]() {
        box->setVisible(false);
        _arBox.erase(box);
        box->deleteLater();
    });
}

void LiteMessageBox::failed(const QString &message, int duration, QWidget *base)
{
    static std::set<LiteMessageBox *> _arBox;

    for (auto *box : _arBox) {
        box->setVisible(false);
    }

    auto *box = new LiteMessageBox(EM_TYPE_FAILED, message, base);
    _arBox.insert(box);
    box->setAttribute(Qt::WA_QuitOnClose, false);
    box->setVisible(true);

    if (duration <= 1000) {
        duration = 1000;
    }

    QTimer::singleShot(duration, [box]() {
        box->setVisible(false);
        _arBox.erase(box);
        box->deleteLater();
    });
}
