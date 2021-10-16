//
// Created by cc on 2019/11/14.
//

#include "FileRecordWnd.h"
#include <QScrollBar>
#include <QtConcurrent>
#include <QPainter>
#include <QGridLayout>
#include <QToolButton>
#include <QFileInfo>
#include "../ChatUtil.h"

#include "../MessageItems/FileRoundProgressBar.h"
#include "../ChatViewMainPanel.h"
#include "CustomUi/TitleBar.h"
#include "Util/ui/StyleDefine.h"
#include "../NetImageLabel.h"
#include "CustomUi/QtMessageBox.h"

extern ChatViewMainPanel *g_pMainPanel;

/** FileRecordItemWgt **/
FileRecordItemWgt::FileRecordItemWgt(const StFileRecord &record,
                                     QWidget *parent)
    : QFrame(parent)
    , _msg_id(record.msg_id.toStdString())
    , _md5(record.file_md5.toStdString())
    , _file_link(record.file_link)
{
    // left main
    auto *contentFrm = new QFrame(this);
    contentFrm->setFixedSize(350, 55);
    contentFrm->setObjectName("borderFrm");
    auto *iconLabel = new NetImageLabel("", this);
    iconLabel->setLocalImage(st::getIconByFileName(record.file_name));
    iconLabel->setFixedSize(36, 36);
    auto *nameLabel = new QLabel(record.file_name, this);
    nameLabel->setMaximumWidth(300);
    auto *sizeLabel = new QLabel(record.file_size, this);
    nameLabel->setObjectName("FileItemWgtFileNameLabel");
    sizeLabel->setObjectName("FileItemWgtFileSizeLabel");
    auto *contentLay = new QGridLayout(contentFrm);
    contentLay->addWidget(iconLabel, 0, 0, 2, 1);
    contentLay->addWidget(nameLabel, 0, 1, 1, 2);
    contentLay->addWidget(sizeLabel, 1, 1, 1, 1);
    contentLay->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding)
                        , 1, 2, 1, 1);
    // button
    auto *downloadBtn = new QToolButton(this);
    downloadBtn->setObjectName("FileItemWgt_downloadBtn");
    downloadBtn->setToolTip(tr("下载"));
    auto *openPathBtn = new QToolButton(this);
    openPathBtn->setObjectName("FileItemWgt_openPathBtn");
    openPathBtn->setToolTip(tr("打开文件夹"));
    auto *processBar = new FileRoundProgressBar(this);
    openPathBtn->setFixedSize(26, 26);
    downloadBtn->setFixedSize(26, 26);
    processBar->setFixedSize(26, 26);
    auto *mainLay = new QHBoxLayout;
    mainLay->addWidget(contentFrm);
    mainLay->addItem(new QSpacerItem(10, 10, QSizePolicy::Expanding));
    mainLay->addWidget(processBar);
    mainLay->addWidget(downloadBtn);
    mainLay->addWidget(openPathBtn);
    //
    auto *sourceLabel = new QLabel(this);
    sourceLabel->setMaximumWidth(300);
    sourceLabel->setObjectName("RecordFileTip");
    sourceLabel->setText(tr("来自: %1").arg(record.source));
    auto *lay = new QVBoxLayout(this);
    lay->setContentsMargins(20, 10, 20, 10);
    lay->addLayout(mainLay);
    lay->addWidget(sourceLabel);
    //
    auto localPath = st::File::getRealFilePath(record.msg_id.toStdString(),
                                               record.file_md5.toStdString());
    bool downloaded = !localPath.isEmpty();
    processBar->setVisible(false);
    downloadBtn->setVisible(!downloaded);
    openPathBtn->setVisible(downloaded);
    connect(openPathBtn, &QToolButton::clicked, this,
            &FileRecordItemWgt::onOpenFilePath);
    connect(downloadBtn, &QToolButton::clicked, this, [this]() {
        QDesktopServices::openUrl(QUrl(_file_link));
    });
}

void FileRecordItemWgt::onOpenFilePath()
{
    QString fileName = st::File::getRealFilePath(_msg_id, _md5);
    QFileInfo info(fileName);

    if (fileName.isEmpty() || !info.exists()) {
        int ret = QtMessageBox::question(this, tr("提醒"),
                                         tr("未找到本地文件, 是否下载?"));

        if (ret == QtMessageBox::EM_BUTTON_NO) {
            return;
        } else {
            QDesktopServices::openUrl(QUrl(_file_link));
        }
    } else {
        st::File::openFileFolder(fileName);
    }
}

/** FileRecordDelegate **/
FileRecordDelegate::FileRecordDelegate(QListView *parent)
    : QStyledItemDelegate(parent), _pParentView(parent)
{
}

void FileRecordDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    painter->save();
    painter->setRenderHint(QPainter::TextAntialiasing);
    QRect rect = option.rect;

    if (option.state & QStyle::State_Selected) {
        painter->fillRect(rect, st::StyleDefine::instance().getSearchSelectColor());
    } else {
        painter->fillRect(rect, st::StyleDefine::instance().getSearchNormalColor());
    }

    auto *pThis = const_cast<FileRecordDelegate *>(this);

    if (pThis) {
        pThis->dealWidget(option, index);
    }

    painter->restore();
}

QSize FileRecordDelegate::sizeHint(const QStyleOptionViewItem &option,
                                   const QModelIndex &index) const
{
    //    auto size = QStyledItemDelegate::sizeHint(option, index);
    return {_pParentView->width(), 100};
}

void FileRecordDelegate::dealWidget(const QStyleOptionViewItem &option,
                                    const QModelIndex &index)
{
    QWidget *indexWgt = _pParentView->indexWidget(index);

    if (indexWgt) {
    } else {
        auto *itemBase = creatWgt(option, index);
        _pParentView->setIndexWidget(index, itemBase);
    }
}

QWidget *FileRecordDelegate::creatWgt(const QStyleOptionViewItem &option,
                                      const QModelIndex &index)
{
    StFileRecord record = index.data(EM_FILE_RECORD_INFO).value<StFileRecord>();
    auto *wgt = new FileRecordItemWgt(record);
    return wgt;
}

/** FileRecordWnd **/
FileRecordWnd::FileRecordWnd(QWidget *parent)
    : UShadowDialog(parent, true)
{
    initUI();
    setFixedWidth(500);
    setMinimumHeight(720);

    QPointer<FileRecordWnd> that(this);
    auto searchFun = [that](lazyq<QString>::lazyqq & q) {
        if (q->empty() || !that) {
            return;
        }

        QString key = q->back();
        emit that->sgGoSearch(key);
    };
    _searchQueue = new lazyq<QString>(300, searchFun);
}

void FileRecordWnd::initUI()
{
#ifdef _MACOS
    setWindowFlags(this->windowFlags() | Qt::Tool);
#endif
    auto *titleBar = new TitleBar(tr("文件管理器"), this, this);
    titleBar->setFixedHeight(50);
    setMoverAble(true, titleBar);
    //
    auto *searchFrm = new QFrame(this);
    searchFrm->setObjectName("searchFrm");
    auto *searchLay = new QHBoxLayout(searchFrm);
    {
        searchLay->setMargin(16);
        _pSearchEdit = new Search_Edit(this);
        _pSearchEdit->setFixedHeight(30);
        searchLay->addWidget(_pSearchEdit);
    }
    _pView = new QListView(this);
    _pView->setFrameShape(QFrame::NoFrame);
    _pModel = new QStandardItemModel(this);
    _pView->setModel(_pModel);
    _pView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _pView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    _pView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _pView->verticalScrollBar()->setSingleStep(12);
    auto *itemDelegate = new FileRecordDelegate(_pView);
    _pView->setItemDelegate(itemDelegate);
    _pLoading = st::makeLoadingLabel();
    _pLoadingBottom = st::makeLoadingLabel(true, {50, 50});
    _pLoadingBottom->setVisible(false);
    _pStackWgt = new QStackedWidget(this);
    _pStackWgt->addWidget(_pView);
    _pStackWgt->setCurrentWidget(_pView);
    _pStackWgt->addWidget(_pLoading);
    auto *mainFrm = new QFrame(this);
    mainFrm->setObjectName("MainFrm");
    auto *mainLay = new QVBoxLayout(mainFrm);
    mainLay->setContentsMargins(0, 0, 0, 6);
    mainLay->setSpacing(0);
    mainLay->addWidget(searchFrm);
    mainLay->addWidget(_pStackWgt);
    mainLay->addWidget(_pLoadingBottom);
    auto *lay = new QVBoxLayout(_pCenternWgt);
    lay->setMargin(0);
    lay->setSpacing(0);
    lay->addWidget(titleBar);
    lay->addWidget(mainFrm);
    connect(_pSearchEdit, &Search_Edit::textChanged,
    this, [this](const QString & text) {
        _searchQueue->push(text);
    });
    connect(this, &FileRecordWnd::sgGoSearch, this, &FileRecordWnd::searchFile);
    connect(this, &FileRecordWnd::sgGotResult, this, &FileRecordWnd::updateUi);
    connect(_pView->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &FileRecordWnd::onScrollChanged);
}

void FileRecordWnd::setSearch(const QString &key)
{
    if (_pSearchEdit) {
        if (_pSearchEdit->text() != key) {
            _pSearchEdit->setText(key);
        } else {
            searchFile(key);
        }
    }
}

void FileRecordWnd::searchFile(const QString &key)
{
    _hasMore = false;
    _pModel->clear();
    _key = key.toStdString();
    _pStackWgt->setCurrentWidget(_pView);
    _pos = 0;
    _scroll_value = 0;
    //
    _pLoading->movie()->start();
    _pStackWgt->setCurrentWidget(_pLoading);
    //
    goSearch();
}

//
void FileRecordWnd::goSearch()
{
    using namespace st::Search;
    _hasMore = false;

    if (g_pMainPanel) {
        QtConcurrent::run([this]() {
            SearchInfoEvent searchInfo;
            searchInfo.start = _pos;
            searchInfo.length = 15;
            _pos += 15;
            searchInfo.key = _key;
            searchInfo.action = EM_ACTION_HS_FILE;
            ChatMsgManager::sendSearch(searchInfo);
            _fileRecords.clear();

            for (const auto &it : searchInfo.searchRet) {
                _hasMore |= it.second.hasMore;
                const auto history = it.second._files;
                _fileRecords.insert(_fileRecords.end(), history.begin(), history.end());
            }

            emit sgGotResult( searchInfo.key.data());
        });
    }
}

void FileRecordWnd::updateUi(const QString &key)
{
    if (_key.data() != key) {
        return;
    }

    for (const auto &it : _fileRecords) {
        auto *item = new QStandardItem;
        StFileRecord record;
        //        record.from_name = it.;
        record.msg_id = it.msg_id.data();
        record.source = it.source.data();
        record.file_name = it.file_name.data();
        record.file_md5 = it.file_md5.data();
        record.file_size = it.file_size.data();
        record.file_link = it.file_url.data();
        item->setData(QVariant::fromValue(record), EM_FILE_RECORD_INFO);
        item->setText(it.file_name.data());
        item->setToolTip(it.file_name.data());
        _pModel->appendRow(item);
    }

    _pStackWgt->setCurrentWidget(_pView);
    _pLoading->movie()->stop();
    _pLoadingBottom->movie()->stop();
    _pLoadingBottom->setVisible(false);
}

//
void FileRecordWnd::onScrollChanged(int value)
{
    // no more -> return
    if (!_hasMore) {
        return;
    }

    // scroll bar direction
    if (value <= _scroll_value) {
        return;
    }

    //
    _scroll_value = value;
    auto max = _pView->verticalScrollBar()->maximum();

    if (max - value <= 5) {
        // less then 500 ms -> return
        static qint64 req_time = 0;
        auto now = QDateTime::currentMSecsSinceEpoch();

        if (now - req_time < 500) {
            return;
        }

        req_time = now;
        // show loading movie
        _pLoadingBottom->movie()->start();
        _pLoadingBottom->setVisible(true);
        // request
        goSearch();
    }
}
