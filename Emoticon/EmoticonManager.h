#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
#ifndef _EMOTICONMANAGER_H_
#define _EMOTICONMANAGER_H_

#include "CustomUi/UShadowWnd.h"
#include <QFrame>
#include "EmoticonStruct.h"

class LocalEmoticon;
class NetEmoticon;
class EmoticonManager;

class EmoticonManager : public UShadowDialog
{
	Q_OBJECT

public:
	EmoticonManager(QMap<QString, StEmoticon>& mapEmo, QWidget* parent);
	~EmoticonManager() override;

public:
	LocalEmoticon* getLocalManager() { return _pLocalEmoManager; }

private:
	void initUi();
	void onBtnClicked();

private:
	LocalEmoticon* _pLocalEmoManager;
	NetEmoticon*   _pNetEmoManager;
	QPushButton*   _pBtnLocalEmo;
	QPushButton*   _pBtnNetEmo;
private:
	QMap<QString, StEmoticon>& _mapEmos;
};

#endif
