#include "uicom.h"

UICom *UICom::getInstance()
{
    static UICom ui;
    return &ui;
}

QWidget *UICom::getAcltiveMainWnd()
{
    return _pCurActiveWnd;
}

void UICom::setAcltiveMainWnd(QWidget *wnd)
{
    if (wnd) {
        _pCurActiveWnd = wnd;
    }
}
