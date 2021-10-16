//
// Created by cc on 18-12-14.
//

#ifndef STALK_V2_SEARCHTHREAD_H
#define STALK_V2_SEARCHTHREAD_H

#include <QThread>
#include <QMutexLocker>
#include "Util/lazy/lazyq.h"

class SearchResultPanel;

class SearchThread : public QObject
{
    Q_OBJECT
public:
    SearchThread();

public:
    void addSearchReq(const QString &req);

Q_SIGNALS:

    void sgSearchStart(const QString &);

protected:
    void run();

private:
    lazyq<bool> *_searchQueue;

    QString _lastReq;
};


#endif //STALK_V2_SEARCHTHREAD_H
