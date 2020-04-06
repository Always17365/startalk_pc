﻿#ifndef _TCPDISCONNECT_H_
#define _TCPDISCONNECT_H_
#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
#include <QFrame>

class NavigationMianPanel;
class QLabel;
class TcpDisconnect : public QFrame
{
	Q_OBJECT

public:
	explicit TcpDisconnect(NavigationMianPanel* pMainPanel, QWidget *parent = nullptr);
	~TcpDisconnect() override;

public:
	void onRetryConnected();
	void setText(const QString& text);

//private:
//	void mousePressEvent(QMouseEvent *e) override;

Q_SIGNALS:
    void sgSetText(const QString&);

private:
	NavigationMianPanel* _pMainPanel;
	QLabel*              _pTextLabel;
};

#endif // _TCPDISCONNECT_H_

