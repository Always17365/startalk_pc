#ifndef _USERMAGANER_H_
#define _USERMAGANER_H_

#include <map>
#include <vector>
#include "include/CommonStrcut.h"
#include "entity/im_user.h"
#include "entity/im_userSupplement.h"
#include "Util/Entity/JID.h"

typedef std::map<std::string, std::map<std::string, int>> UserCardParam; //<domain<userid, version>>

class Communication;
class UserManager
{
public:
    explicit UserManager(Communication* pComm);
	~UserManager();

public:
	// 获取组织架构
	bool getNewStructure(bool = false);
//	bool getOldStructure();
	//
	bool getUserCard(const UserCardParam &param, std::vector<st::StUserCard>& arUserInfo);

	void getUserFullInfo(std::shared_ptr<st::entity::ImUserSupplement>& imUserSup,
                         std::shared_ptr<st::entity::ImUserInfo>& userInfo);

	bool getPhoneNo(const std::string& userId, std::string& phoneNo);

	bool changeUserHead(const std::string& headPath);
    void UpdateMood(const std::string& mood);

private:
//	bool getUserMood(st::Entity::JID* jid, std::string& mood, int& version);
	bool getUserSupplement(st::entity::JID *jid, std::shared_ptr<st::entity::ImUserSupplement>& imUserSup);

private:
	Communication* _pComm;
};

#endif//_USERMAGANER_H_