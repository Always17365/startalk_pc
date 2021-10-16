//
// Created by cc on 19-1-9.
//

#ifndef STALK_V2_ATMESSAGETIP_H
#define STALK_V2_ATMESSAGETIP_H

#include <QFrame>
#include <QListWidget>
#include <QLabel>
#include <QStandardItem>
#include <QMutexLocker>
#include <utility>

struct StShowAtInfo {
    bool            isAtAll;
    QString          name;
    QStandardItem*  atItem{};

    StShowAtInfo(): isAtAll(false) {}
    StShowAtInfo(bool all, const QString& n, QStandardItem* item)
        : isAtAll(all), name(n), atItem(item)
    {

    }
};

class ChatMainWgt;
class AtMessageTip : public QFrame
{
	Q_OBJECT
public:
    explicit AtMessageTip(ChatMainWgt* chatMainWgt);

public:
    void addAt(const StShowAtInfo& info);
    void showAtInfo();
    void clear();

protected:
    void mousePressEvent(QMouseEvent* e) override;

private:
    ChatMainWgt*      _pChatMainWgt;
    QLabel*           _pNameLabel;
    QLabel*           _pAtLabel;

private:
    QList<StShowAtInfo> _arAtItems;

private:
    QMutex             _mutex;
};


#endif //STALK_V2_ATMESSAGETIP_H
