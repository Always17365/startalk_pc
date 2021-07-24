#ifndef QTALK_V2_PROCESS_INFO_H
#define QTALK_V2_PROCESS_INFO_H

#include <QThread>

class ProcessInfo : public QThread {
	Q_OBJECT
public:
	ProcessInfo(QObject* parent = nullptr);
	~ProcessInfo();

public:

private:
	void run();

private:
	bool _run{};
};

#endif // !QTALK_V2_PROCESS_INFO_H