#include "lazyq.h"

#include <QDateTime>
#include "lazyqHelper.h"
#include "Util/utils.h"
#include "Util/Log.h"

lazyqImpl::lazyqImpl(int delayMs, const LazyImplCallback &func)
    : _callback(func)
{
    _delay = qMax(delayMs, 100);
    _uuid = st::utils::uuid();
    _deal.store(true);
}

void lazyqImpl::addTask()
{
    if (!_deal.load()) {
        LazyQHelper::instance()->removeTask(_t, _uuid);
    }

    _deal.store(false);

    auto now = QDateTime::currentMSecsSinceEpoch();
    _t = now + _delay;

    LazyQHelper::instance()->addTask(_t, _uuid,
                                     std::bind(&lazyqImpl::helperCallback, this));
}

void lazyqImpl::helperCallback()
{
    _deal.store(true);
    _callback();
}
