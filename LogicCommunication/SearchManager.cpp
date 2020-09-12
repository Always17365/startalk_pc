﻿#include "SearchManager.h"
#include <iostream>
#include <sstream>
#include "../Platform/NavigationManager.h"
#include "../Platform/Platform.h"
#include "../QtUtil/lib/cjson/cJSON_inc.h"
#include "Communication.h"
#include "../QtUtil/Utils/Log.h"

using namespace QTalk;

SearchManager::SearchManager(Communication *pComm) :
        _pComm(pComm) {

}

/**
  * @函数名
  * @功能描述
  * @参数
  * @date 2018.11.08
  */
void SearchManager::GetSearchResult(SearchInfoEvent &e) {

    std::string searchUrl = NavigationManager::instance().getSearchUrl();
    std::string userId = PLAT.getSelfXmppId();

    cJSON *gObj = cJSON_CreateObject();
    cJSON_AddNumberToObject(gObj, "start", e.start);
    cJSON_AddNumberToObject(gObj, "length", e.length);
    cJSON_AddStringToObject(gObj, "key", e.key.c_str());
    cJSON_AddStringToObject(gObj, "qtalkId", userId.c_str());
    cJSON_AddNumberToObject(gObj, "action", e.action);
    if (!e.to_user.empty())
        cJSON_AddStringToObject(gObj, "to_user", e.to_user.data());
    else if(!e.to_muc.empty())
        cJSON_AddStringToObject(gObj, "to_muc", e.to_muc.data());
    std::string postData = QTalk::JSON::cJSON_to_string(gObj);
    cJSON_Delete(gObj);

    auto callback = [&e](int code, const std::string &responseData) {

        cJSON* response = cJSON_Parse(responseData.data());
        if(nullptr == response)
        {
            error_log("search error retData:{0}", responseData);
            return;
        }

        if (code == 200) {

            cJSON_bool ret = JSON::cJSON_SafeGetBoolValue(response, "ret");
            if(ret)
            {
                cJSON* data = cJSON_GetObjectItem(response, "data");
                cJSON *item = nullptr;
                //
                cJSON_ArrayForEach(item, data) {

                    cJSON* info = cJSON_GetObjectItem(item, "info");
                    if(nullptr == info) continue;

                    Search::StSearchResult tmpRet = Search::StSearchResult();
                    tmpRet.resultType = JSON::cJSON_SafeGetIntValue(item, "resultType");
//                    tmpRet.default_portrait = JSON::cJSON_SafeGetStringValue(item, "defaultportrait");
//                    tmpRet.groupId = JSON::cJSON_SafeGetStringValue(item, "groupId");
                    tmpRet.groupLabel = JSON::cJSON_SafeGetStringValue(item, "groupLabel");
                    tmpRet.hasMore = JSON::cJSON_SafeGetBoolValue(item, "hasMore");

                    if(tmpRet.resultType & Search::EM_ACTION_USER)
                    {
                        cJSON *iitem = nullptr;
                        cJSON_ArrayForEach(iitem, info) {
                            Search::StUserItem tmpItem;
                            tmpItem.xmppId = JSON::cJSON_SafeGetStringValue(iitem, "uri");
                            tmpItem.name = JSON::cJSON_SafeGetStringValue(iitem, "name");
                            tmpItem.tips = JSON::cJSON_SafeGetStringValue(iitem, "label");
                            tmpItem.icon = JSON::cJSON_SafeGetStringValue(iitem, "icon");
                            tmpItem.structure = JSON::cJSON_SafeGetStringValue(iitem, "content");

                            tmpRet._users.push_back(tmpItem);
                        }
                    }
                    else if((tmpRet.resultType & Search::EM_ACTION_MUC)
                        || (tmpRet.resultType & Search::EM_ACTION_COMMON_MUC))
                    {
                        cJSON *iitem = nullptr;
                        cJSON_ArrayForEach(iitem, info) {
                            Search::StGroupItem tmpItem;

                            cJSON* hits = cJSON_GetObjectItem(iitem, "hit");
                            tmpItem.type = JSON::cJSON_SafeGetIntValue(iitem, "todoType");
                            tmpItem.xmppId = JSON::cJSON_SafeGetStringValue(iitem, "uri");
                            tmpItem.name = JSON::cJSON_SafeGetStringValue(iitem, "label");
                            tmpItem.icon = JSON::cJSON_SafeGetStringValue(iitem, "icon");

                            if(nullptr != hits)
                            {
                                cJSON *iiitem = nullptr;
                                cJSON_ArrayForEach(iiitem, hits) {
                                    tmpItem._hits.emplace_back(iiitem->valuestring);
                                }
                            }

                            tmpRet._groups.push_back(tmpItem);
                        }
                    }
                    else if((tmpRet.resultType & Search::EM_ACTION_HS_SINGLE) ||
                            (tmpRet.resultType & Search::EM_ACTION_HS_MUC))
                    {
                        cJSON *iitem = nullptr;
                        cJSON_ArrayForEach(iitem, info) {
                            Search::StHistory tmpItem;

                            tmpItem.key = e.key;
                            tmpItem.type = JSON::cJSON_SafeGetIntValue(iitem, "todoType");
                            tmpItem.name = JSON::cJSON_SafeGetStringValue(iitem, "label");
                            tmpItem.icon = JSON::cJSON_SafeGetStringValue(iitem, "icon");
                            if (cJSON_HasObjectItem(iitem, "count"))
                                tmpItem.count = JSON::cJSON_SafeGetIntValue(iitem, "count");
                            tmpItem.body = JSON::cJSON_SafeGetStringValue(iitem, "body");
                            tmpItem.time = atoll(JSON::cJSON_SafeGetStringValue(iitem, "time"));
                            tmpItem.from = JSON::cJSON_SafeGetStringValue(iitem, "from");
                            tmpItem.to = JSON::cJSON_SafeGetStringValue(iitem, "to");
                            tmpItem.msg_type = atoi(JSON::cJSON_SafeGetStringValue(iitem, "mtype"));
                            tmpItem.msg_id = JSON::cJSON_SafeGetStringValue(iitem, "msgid");
                            tmpItem.extend_info = JSON::cJSON_SafeGetStringValue(iitem, "extendinfo");
//                            tmpItem.real_from = JSON::cJSON_SafeGetStringValue(iitem, "realfrom");
//                            tmpItem.real_to = JSON::cJSON_SafeGetStringValue(iitem, "realto");

                            tmpRet._history.push_back(tmpItem);
                        }
                    }
                    else if((tmpRet.resultType & Search::EM_ACTION_HS_FILE)) {
                        cJSON *iitem = nullptr;
                        cJSON_ArrayForEach(iitem, info) {
                            Search::StHistoryFile tmpItem;

                            tmpItem.key = e.key;
                            tmpItem.source = JSON::cJSON_SafeGetStringValue(iitem, "source");
                            tmpItem.msg_id = JSON::cJSON_SafeGetStringValue(iitem, "msgid");
//                            tmpItem.icon = JSON::cJSON_SafeGetStringValue(iitem, "icon");
                            tmpItem.body = JSON::cJSON_SafeGetStringValue(iitem, "body");
                            tmpItem.extend_info = JSON::cJSON_SafeGetStringValue(iitem, "extendinfo");
                            tmpItem.time = atoll(JSON::cJSON_SafeGetStringValue(iitem, "time"));
                            tmpItem.from = JSON::cJSON_SafeGetStringValue(iitem, "from");
                            tmpItem.to = JSON::cJSON_SafeGetStringValue(iitem, "to");

                            cJSON* file_item = cJSON_GetObjectItem(iitem, "fileinfo");

                            tmpItem.file_md5 = JSON::cJSON_SafeGetStringValue(file_item, "FILEMD5");
                            tmpItem.file_name = JSON::cJSON_SafeGetStringValue(file_item, "FileName");
                            tmpItem.file_size = JSON::cJSON_SafeGetStringValue(file_item, "FileSize");
                            tmpItem.file_url = JSON::cJSON_SafeGetStringValue(file_item, "HttpUrl");

                            tmpRet._files.push_back(tmpItem);
                        }
                    }
                    e.searchRet[tmpRet.resultType] = tmpRet;
                }
            }
        } else {
            std::string errmsg = JSON::cJSON_SafeGetStringValue(response, "errmsg");
            error_log("search error errmsg:{0}", errmsg);
        }

        cJSON_Delete(response);
    };

    if (_pComm) {
        QTalk::HttpRequest req(searchUrl, RequestMethod::POST);
        req.header["Content-Type"] = "application/json;";
        req.body = postData;
        _pComm->addHttpRequest(req, callback);
    }
}
