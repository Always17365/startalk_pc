#ifndef STATICIMAGEMESSITEM_H
#define STATICIMAGEMESSITEM_H

#include "MessageItemBase.h"

#include <QLabel>
#include <QTimer>



class ImageMessItem : public MessageItemBase
{
    Q_OBJECT
public:
    explicit ImageMessItem(const StNetMessageResult &msgInfo,
                           const QString &link,
                           const QString &path,
                           const QSizeF &size,
                           QWidget *parent = nullptr);
    // MessageItemBase interface
public:
    QSize itemWdtSize() override;
    void onImageDownloaded();

    // QWidget interface
protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *e) override;
    bool event(QEvent *e) override ;

private:
    void init();
    void initLayout();
    void initSendLayout();
    void initReceiveLayout();
    void setImage();

private slots:
    void onMoveTimer();
    void onMovieChanged(int);

Q_SIGNALS:
    void sgItemChanged();

private:
    QLabel *_imageLab{ nullptr };
    QMovie *_movie {nullptr};
    bool    _isGIF {false};

    QSize _headPixSize;
    QMargins _mainMargin;
    QMargins _leftMargin;
    QMargins _rightMargin;
    QMargins _contentMargin;
    QSize _sizeMaxPix;

    int _mainSpacing;
    int _leftSpacing;
    int _rightSpacing;
    int _contentSpacing;
    int _nameLabHeight;

    QTimer _moveTimer;

public:
    QString _imagePath;
    QString _imageLink;
    QSize   _size;
    qint64   _paintTime = 0;
};

#endif // STATICIMAGEMESSITEM_H
