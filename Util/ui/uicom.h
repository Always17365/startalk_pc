#ifndef UICOM_H
#define UICOM_H

#include "Util/util_global.h"
#include <QWidget>
#include <QMutexLocker>

class STALK_UTIL_EXPORT UICom
{
public:
    static  UICom *getInstance();

public:
    QWidget *getAcltiveMainWnd();
    void setAcltiveMainWnd(QWidget *wnd);

private:
    UICom() = default;

private:
    QWidget *_pCurActiveWnd{nullptr};
};

#endif // UICOM_H
