//
// Created by cc on 18-12-18.
//

#ifndef STALK_V2_FRIENDLISTDAO_H
#define STALK_V2_FRIENDLISTDAO_H


#include "DaoInterface.h"
#include <vector>
#include "entity/im_friend_list.h"

class FriendListDao : public DaoInterface
{
public:
    explicit FriendListDao(st::sqlite::database *sqlDb);
    bool creatTable() override;

public:
    bool bulkInsertFriends(const std::vector<st::entity::IMFriendList>& friends);
    bool insertFriend(st::entity::IMFriendList imfriend);
    bool deleteAllFriends();
    bool deleteFriendByXmppId(const std::string& xmppId);
    bool getAllFriends(std::vector<st::entity::IMFriendList>& friends);


};


#endif //STALK_V2_FRIENDLISTDAO_H
