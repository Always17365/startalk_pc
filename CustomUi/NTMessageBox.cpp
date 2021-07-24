//
// Created by cc on 2020/2/12.
//

#include "NTMessageBox.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QKeyEvent>
#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDebug>
#include <QSharedPointer>

NTMessageBox::NTMessageBox(QWidget *parent, const QString &message, int buttons)
    : UShadowDialog(parent, true), _retButton(EM_BUTTON_INVALID)
{
    setFixedWidth(298);
    mainMessageLabel = new QLabel(this);
    //
    mainMessageLabel->setAlignment(Qt::AlignTop);
    mainMessageLabel->setObjectName("MessageBoxMessage");
    //
    auto *leftLay = new QVBoxLayout;
    leftLay->setMargin(0);
    leftLay->addItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));
    //
    auto *rightLay = new QVBoxLayout;
    rightLay->setMargin(0);
    rightLay->addWidget(mainMessageLabel);
    //
    auto *buttonLay = new QHBoxLayout;
    buttonLay->addItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));

    if(buttons & EM_BUTTON_NO)
    {
        auto *noButton = new QPushButton(tr("取消"), this);
        noButton->setObjectName("NoButton");
        buttonLay->addWidget(noButton);
        noButton->setFixedSize(72, 32);
        connect(noButton, &QPushButton::clicked, this, [this]()
        {
            _retButton = EM_BUTTON_NO;
            this->setVisible(false);
            _evtLoop->exit();
        });
    }

    if(buttons & EM_BUTTON_YES)
    {
        auto *yesButton = new QPushButton(tr("确定"), this);
        yesButton->setObjectName("YesButton");
        buttonLay->addWidget(yesButton);
        yesButton->setFixedSize(72, 32);
        connect(yesButton, &QPushButton::clicked, this, [this]()
        {
            _retButton = EM_BUTTON_YES;
            this->setVisible(false);
            _evtLoop->exit();
        });
    }

    rightLay->addLayout(buttonLay);
    auto *mainFrm = new QFrame(this);
    mainFrm->setObjectName("MessageBox");
    auto *mainLay = new QHBoxLayout(mainFrm);
    mainLay->setContentsMargins(32, 32, 32, 20);
    mainLay->setSpacing(16);
    mainLay->addLayout(leftLay);
    mainLay->addLayout(rightLay);
    auto *lay = new QHBoxLayout(_pCenternWgt);
    lay->addWidget(mainFrm);
    lay->setMargin(0);
    setMoverAble(true);
    _evtLoop = new QEventLoop(this);
    mainMessageLabel->setText(message);
#ifdef _MACOS
    macAdjustWindows();
#endif
}

NTMessageBox::~NTMessageBox()
{
    qInfo() << mainMessageLabel->text() << "delete";
}

void NTMessageBox::keyPressEvent(QKeyEvent *e)
{
    if(e == QKeySequence::Copy)
    {
        QString text = QString("%1").arg(mainMessageLabel->text());
        QApplication::clipboard()->setText(text);
    }
    else if(e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
    {
        _retButton = EM_BUTTON_YES;
        this->setVisible(false);
        _evtLoop->exit();
    }
    else if(e->key() == Qt::Key_Escape || e->key() == Qt::Key_Space)
    {
        _retButton = EM_BUTTON_NO;
        this->setVisible(false);
        _evtLoop->exit();
    }

#ifdef _MACOS

    if(e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_W)
    {
        _retButton = EM_BUTTON_NO;
        this->setVisible(false);
        _evtLoop->exit();
    }

#endif
    UShadowDialog::keyPressEvent(e);
}

int NTMessageBox::showMessage(QWidget *parent, const QString &message, int buttons)
{
    auto _box = QSharedPointer<NTMessageBox>(new NTMessageBox(parent, message, buttons));
    _box->showModel();
    _box->_evtLoop->exec();
    int ret = _box->_retButton;
    return ret;
}
