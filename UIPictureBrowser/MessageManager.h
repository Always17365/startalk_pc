//
// Created by cc on 2019-02-22.
//

#ifndef STALK_V2_MESSAGEMANAGER_H
#define STALK_V2_MESSAGEMANAGER_H

#include "EventBus/EventHandler.hpp"
#include "Message/LoginMessgae.h"
#include "EventBus/HandlerRegistration.hpp"
#include "Message/UserMessage.h"
#include <vector>

class SearchInfoEvent;
class PictureMsgManager : public Object
{
public:
    static std::string getSourceImage(const std::string& netPath);
    static void getBeforeImgMessages(const std::string& messageId, std::vector<std::pair<std::string, std::string>> & msgs);
    static void getNextImgMessages(const std::string& messageId, std::vector<std::pair<std::string, std::string>> & msgs);
};

//class PictureBrowser;
//class PictureMsgListener
//{
//public:
//    explicit PictureMsgListener(PictureBrowser *pMainPanel);
//    virtual ~PictureMsgListener();
//
//public:
//
//private:
//    PictureBrowser* _pMainPanel;
//};

#endif //STALK_V2_MESSAGEMANAGER_H
