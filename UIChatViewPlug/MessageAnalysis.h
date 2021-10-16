//
// Created by cc on 2019/11/07.
//

#ifndef STALK_V2_MESSAGEANALYSIS_H
#define STALK_V2_MESSAGEANALYSIS_H

#include "search/SearchItemWgt.h"

namespace st
{
    void analysisMessage(StNetMessageResult &info);
    // text
    void analysisTextMessage(const QString& content, std::vector<StTextMessage>& messages,
            const std::map<QString, QString>& atUsers = {});
    void analysisSendTextMessage(const QString& extendInfo, std::vector<StTextMessage>& messages);
    // link
    void analysisLinkMessage(const QString &text, std::vector<StTextMessage>& messages,
            const std::map<QString, QString>& atUsers);
    // at
    void analysisAtMessage(const QString &text, std::vector<StTextMessage>& messages,
            const std::map<QString, QString>& atUsers);
    // image
    void analysisImageMessage(const QString& content, QString& imageLink, qreal& width, qreal& height);
    void analysisEmoticonMessage(const QString& content, QString& pkgid, QString& shortCut);
    // file
    void analysisFileMessage(const QString& content, const QString& extendInfo, StNetMessageResult& ret);
    // CommonTrd
    void analysisCommonTrdMessage(const QString& content, const QString& extendInfo, StNetMessageResult& ret);
    // source code
    void analysisCodeMessage(const QString& content, const QString& extendInfo, StNetMessageResult& ret);
    // video item
    void analysisVideoMessage(const QString& content, const QString& extendInfo, StNetMessageResult& ret);
    //
    void analysisLocalImage(const QString& content, QStringList& links);
    void analysisLocalLink(const QString& content, QStringList& links);
}


#endif //STALK_V2_MESSAGEANALYSIS_H
