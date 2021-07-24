#ifndef FILESENDRECEIVEMESSITEM_H
#define FILESENDRECEIVEMESSITEM_H

#include "MessageItemBase.h"

#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include "FileRoundProgressBar.h"

class FileSendReceiveMessItem : public MessageItemBase
{
    Q_OBJECT
public:
    explicit FileSendReceiveMessItem(const StNetMessageResult &msgInfo, QWidget *parent = nullptr);
    ~FileSendReceiveMessItem() override = default;
    // QWidget interface
public:
    QSize itemWdtSize() override;

public slots:
    void setProcess(double speed, double dtotal, double dnow, double utotal, double unow);
    void onSaveAsAct();
    void onUploadFailed();
    void onDownloadFailed();
    void downloadOrUploadSuccess();

Q_SIGNALS:
    void sgOpenDir();
    void sgOpenFile(bool);

private:
    void init();
    void initFileIconMap();
    void initMenu();
    void initLayout();
    void initSendLayout();
    void initReceiveLayout();
    void initSendContentLayout();
    void initReceiveContentLayout();
    void initSendContentTopFrmLayout();
    void initSendContentButtomFrmLayout();
    void initReceiveContentTopFrmLayout();
    void initReceiveContentButtomFrmLayout();
    void setData();
    void judgeFileIsDownLoad();
    void judgeFileIsUpLoad();
    //void sendDownLoadFile(const std::string &strLocalPath, const std::string &strUri);
    void sendNDownLoadFile(const QString &strUri, const QString &strLocalPath);
    void downLoadFile();
    QString getLocalFilePath();

private slots:
    void onDownLoadClicked();
    void onMenuBtnChicked();
    void onOpenFilePath();
    void onOpenFile(bool = false);

private:
    QFrame       *_contentTopFrm{nullptr};
    HeadPhotoLab *_contentTopFrmIconLab{nullptr};
    QLabel       *_contentTopFrmFileNameLab{nullptr};
    QLabel       *_contentTopFrmFileSizeLab{nullptr};

    QFrame *_contentButtomFrm{nullptr};
    QLabel *_contentButtomFrmMessLab{nullptr};
    QPushButton *_contentButtomFrmDownLoadBtn{nullptr};
    QToolButton *_contentButtomFrmMenuBtn{nullptr};
    FileRoundProgressBar *_contentButtomFrmProgressBar{nullptr};
    QPushButton *_contentButtomFrmOPenFileBtn{nullptr};

    QMenu   *_downLoadMenu{nullptr};
    QAction *_saveAsAct{nullptr};
    QAction *_openFileAct{nullptr};

    QSize _headPixSize;
    QMargins _mainMargin;
    QMargins _leftMargin;
    QMargins _rightMargin;
    QMargins _contentMargin;
    QMargins _contentTopFrmHlayMargin;
    QMargins _contentButtomFrmHlayMargin;
    QSize _sizeMaxPix;
    QSize _contentSize;
    QSize _btnSzie;
    QSize _contentTopFrmIconLabSize;

    int _mainSpacing{};
    int _leftSpacing{};
    int _rightSpacing{};
    int _contentSpacing{};
    int _nameLabHeight{};
    int _contentButtomFrmHeight{};
    int _contentTopFrmFileNameLabHeight{};
    int _contentTopFrmFileSizeLabHeight{};
    int _contentTopFrmHlaySpacing{};
    QMap<QString, QString> _fileIcons;
    bool isDownLoad{false};
    bool isUpLoad{false};
    bool _openDir{false};
    bool _openFile{false};
};

#endif // FILESENDRECEIVEMESSITEM_H
