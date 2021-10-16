//
// Created by may on 2018/7/20.
//

#ifndef STALK_QTHTTPREQUEST_H
#define STALK_QTHTTPREQUEST_H

#include "Util/util_global.h"

#include "HttpRequest.h"


typedef int(*CallbackFunc)(void *, double, double, double, double);

struct curl_slist;
struct StProcessData;
namespace st {

class STALK_UTIL_EXPORT QtHttpRequest : public st::http::HttpRequest
{

public:
    explicit QtHttpRequest(std::string &url, long timeout = 10L);

    ~QtHttpRequest();

    void setRequestMethod(RequestMethod method) override;

    void addRequestHeader(const char *key, const char *value) override;

    void addRequestHeader(std::string &key, std::string &value) override;

    void appendPostData(const char *body, int len) override;

    bool startSynchronous() override;

    int getResponseCode() override;

    std::string *getResponseData() override;

    void addFromFile(const std::string &filePath) override;

    void setProcessCallback(const std::string &key,
                            std::function<void(StProcessParam)> fun) override;

    void setDownloadSpeed(int speed) override ;

    void enableProxy() override;

private:
    void *_httpCore;
    std::ostringstream _readBuffer;
    std::string buffer;
    curl_slist *_headers;


    StProcessData *_processData = nullptr;
};
};

#endif //STALK_QTHTTPREQUEST_H
