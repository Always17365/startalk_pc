//
// Created by cc on 2020/3/31.
//

#ifndef STALK_V2_WEBENGINEURLREQUESTINTERCEPTOR_H
#define STALK_V2_WEBENGINEURLREQUESTINTERCEPTOR_H

#include <QWebEngineUrlRequestInterceptor>
#include <QObject>

class WebView;
class WebEngineUrlRequestInterceptor : public QWebEngineUrlRequestInterceptor{
    Q_OBJECT
public:
    explicit WebEngineUrlRequestInterceptor(WebView* webView, QObject *parent = nullptr);
    void interceptRequest(QWebEngineUrlRequestInfo &info);

private:
    WebView* pWebview{};
};


#endif //STALK_V2_WEBENGINEURLREQUESTINTERCEPTOR_H
