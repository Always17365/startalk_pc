﻿#if _MSC_VER >= 1600
    #pragma execution_character_set("utf-8")
#endif
#ifndef _LOCALEMOTICON_H_
#define _LOCALEMOTICON_H_

#include <QFrame>
#include "EmoticonStruct.h"

#define DEM_COLLECTION     "MyEmoticon"

class QTableWidget;
class QListWidget;
class QStackedWidget;
class EmoPreviewWgt;
class QListWidgetItem;
class QMenu;
class EmoLstItemWgt;
class LocalEmoticon : public QWidget
{
    Q_OBJECT

public:
    LocalEmoticon(QMap<QString, StEmoticon> &mapEmo, QWidget *parent);
    ~LocalEmoticon() override;

Q_SIGNALS:
    void removeLocalEmoSignal(const QString &, const QString &);
    void removeCollection(const QString &collectionPath);

public:
    void addEmoticon(const StEmoticon &emoInfo);
    //
    void initEmoticon();
    void initCollection(const StEmoticon &colEmo);
    void updateCollection(const std::map<UnorderMapKey, std::string> &collection);

protected:
    void initUi();
    void onLocalEmoLstClick(QListWidgetItem *item);
    void removeLocalEmo();

protected:
    QListWidget    *_pLocalEmoLst  { nullptr } ;
    EmoPreviewWgt  *_pStackEmoView { nullptr };
    QMenu          *_pMenu         { nullptr };

private:
    QMap<QString, StEmoticon>   &_mapEmo;
    QMap<QString, QTableWidget *> _mapEmoView;
    QMap<QListWidgetItem *, EmoLstItemWgt *> _mapEmoItemWgt;
};

#endif
