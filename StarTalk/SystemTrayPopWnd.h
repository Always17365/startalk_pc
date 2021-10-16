//
// Created by cc on 2020/1/13.
//
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
#ifndef STALK_V2_SYSTEMTRAYPOPWND_H
#define STALK_V2_SYSTEMTRAYPOPWND_H

#include "CustomUi/UShadowWnd.h"
#include "entity/UID.h"
#include "entity/UIEntity.h"
#include <QListView>
#include <map>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QStackedWidget>

struct StNewMessageData{
    int                _chatType{};
    st::entity::UID uid;
    QString            name;
    int                unreadCount{};
    qint64             lastUpdateTime{};

    StNewMessageData()= default;
    StNewMessageData(int ct, const st::entity::UID& u, const QString& n, int unc, qint64 t)
        :_chatType(ct), uid(u), name(n), unreadCount(unc), lastUpdateTime(t) {
    }
};

enum {
    EM_SYSTEM_TRAY_CHAT_TYPE = Qt::UserRole + 1,
    EM_SYSTEM_TRAY_USER_ID,
    EM_SYSTEM_TRAY_REAL_JID,
    EM_SYSTEM_TRAY_NAME,
    EM_SYSTEM_TRAY_UNREAD_COUNT,
    EM_SYSTEM_TRAY_LAST_TIME,
};

class ImageTextLabel : public QFrame {
    Q_OBJECT
public:
    ImageTextLabel(const QString& imagePath, const QString& text, QWidget* parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent* e) override ;
    void paintEvent(QPaintEvent *) override ;

Q_SIGNALS:
    void clicked();

private:
    QString _text;
    QString _path;
};

class SystemTrayDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit SystemTrayDelegate(QObject *parent = nullptr);

protected:
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
    bool editorEvent(QEvent *e, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &index) override ;

Q_SIGNALS:
    void sgJumtoSession(const StSessionInfo&);
};

/** **/
class SystemTrayPopWnd : public UShadowDialog{
    Q_OBJECT
public:
    explicit SystemTrayPopWnd(QWidget* parent = nullptr);

public:
    void setCancelAlertBtnVisible(bool show);
    bool hasNewMessage();

public slots:
    void onNewMessage(int, const st::entity::UID&, const QString&, qint64, int);

protected:
    bool eventFilter(QObject* o, QEvent* e) override ;

private:
    void onClearNewMessage();
    void onClicked(const StSessionInfo&);

Q_SIGNALS:
    void sgQuit();
    void sgFeedback();
    void sgCancelAlert();
    void sgStartTimer();
    void sgJumtoSession(const StSessionInfo&);

private:
    QListView           *_pNewMessageView{};
    QStandardItemModel  *_pNewMessageModel{};
    QLabel*             emptyLabel;
    QStackedWidget      *_pStackedWidget{};

    std::vector<StNewMessageData>  _data;
    std::map<st::entity::UID, QStandardItem*> _items;

private:
    QLabel *cancelAlert{};
};


#endif //STALK_V2_SYSTEMTRAYPOPWND_H
