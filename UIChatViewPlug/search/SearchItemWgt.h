//
// Created by cc on 2019/11/06.
//

#ifndef STALK_V2_SEARCHITEMWGT_H
#define STALK_V2_SEARCHITEMWGT_H

#include "ItemWgt.h"
#include <vector>
#include <QFrame>
#include <QToolButton>
#include <QVBoxLayout>
#include "../NativeChatStruct.h"
#include "../MessageItems/TextBrowser.h"

/** SearchItemBase **/
class SearchItemBase : public QFrame
{
    Q_OBJECT
public:
    explicit SearchItemBase(const StNetMessageResult &info,
                            QWidget *parent = nullptr);

public:
    void setDetailButtonVisible(bool);

Q_SIGNALS:
    void sgGetMessageDetail(qint64 time, const QString &, int);

protected:
    NameTitleLabel *title{};
    QToolButton *_pDetailBtn{};
    QVBoxLayout *lay{};
};


/** SearchTextItem **/
class SearchTextItem : public SearchItemBase
{
    Q_OBJECT
public:
    explicit SearchTextItem(const StNetMessageResult &info,
                            QWidget *parent = nullptr);

public:
    QSize getContentSize(qreal width);

protected:
    bool event(QEvent *event) override ;

Q_SIGNALS:
    void sgSelectIndex();

private:
    void downloadImage(const QString &imageLink);
    void downloadEmoticon(const QString &pkgid, const QString &shortCut);

private:
    TextBrowser *_pBrowser = nullptr;

private:
    std::map<QString, QMovie *> _mapMovies;
};


/*** tip item ***/
class SearchTipITem : public SearchItemBase
{
    Q_OBJECT
public:
    explicit SearchTipITem(const StNetMessageResult &info,
                           QWidget *parent = nullptr);

};

/*** file item ***/
class SearchFileITem : public SearchItemBase
{
    Q_OBJECT
public:
    explicit SearchFileITem(const StNetMessageResult &info,
                            QWidget *parent = nullptr);

};

/** CommonTrd item **/
class SearchCommonTrdItem : public SearchItemBase
{
    Q_OBJECT
public:
    explicit SearchCommonTrdItem(const StNetMessageResult &info,
                                 QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *e) override ;

private:
    QFrame *contentFrm{};

private:
    QString _link;
    QString _xmppId;
};

/** code item **/
class SearchCodeItem : public SearchItemBase
{
    Q_OBJECT
public:
    explicit SearchCodeItem(const StNetMessageResult &info,
                            QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *e) override ;

private:
    QFrame *contentFrm{};

private:
    QString _codeStyle;
    QString _codeLanguage;
    QString _code;
};

/** audio video item **/
class SearchAudioVideoItem : public SearchItemBase
{
    Q_OBJECT
public:
    explicit SearchAudioVideoItem(const StNetMessageResult &info,
                                  QWidget *parent = nullptr);
};

/** video item **/
class NetImageLabel;
class SearchVideoItem : public SearchItemBase
{
    Q_OBJECT
public:
    explicit SearchVideoItem(const StNetMessageResult &info,
                             QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *e) override ;

private:
    QString _videoUrl;

    NetImageLabel *imageLabel{};
};
#endif //STALK_V2_SEARCHITEMWGT_H
