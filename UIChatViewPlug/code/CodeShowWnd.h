//
// Created by cc on 2019-02-28.
//
#if _MSC_VER >= 1600
    #pragma execution_character_set("utf-8")
#endif
#ifndef STALK_V2_CODESHOWWND_H
#define STALK_V2_CODESHOWWND_H

#include "CustomUi/UShadowWnd.h"
#include "CodeShell.h"
#include <QWebEngineView>

/**
* @description: CodeShowWnd
* @author: cc
* @create: 2019-02-28 21:52
**/
class ChatViewItem;
class CodeShowWnd : public UShadowDialog
{
    Q_OBJECT
public:
    explicit CodeShowWnd(QWidget *parent = nullptr);
    ~CodeShowWnd() override;

public:
    void showCode(const QString &type, const QString &language, const QString &content);

private:
    void initUi();
    void initRes();
    void loadCodeFile(const QString &type, const QString &language);

private:
    CodeShell      *_pCodeShell{nullptr};
    QWebEngineView *_pWebView  {nullptr};

private:
    QString _strCodeContent;

};



#endif //STALK_V2_CODESHOWWND_H
