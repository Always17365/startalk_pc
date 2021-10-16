//
// Created by cc on 18-12-14.
//

#include "SearchThread.h"
#include <QDateTime>
#include "SearchResultPanel.h"
#include "Util/Log.h"


SearchThread::SearchThread()
{
    auto callback = [this](lazyq<bool>::lazyqq &) {
        emit sgSearchStart(_lastReq);
    };

    _searchQueue = new lazyq<bool>(40, callback);
}

void SearchThread::addSearchReq(const QString &req)
{
    _lastReq = req;
    _searchQueue->push(true);
}
