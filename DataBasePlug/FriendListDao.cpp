//
// Created by cc on 18-12-18.
//

#include "FriendListDao.h"
#include "Util/Log.h"

FriendListDao::FriendListDao(st::sqlite::database *sqlDb)
        : DaoInterface(sqlDb, "IM_Friend_List") {

}

bool FriendListDao::creatTable() {

    if (!_pSqlDb) {
        return false;
    }

    std::string sql = "CREATE TABLE IF NOT EXISTS `IM_Friend_List` ( "
                      "`UserId`       TEXT,"
                      "`XmppId`       TEXT,"
                      "`Version`         INTEGER default 0,"
                      "`ExtendedFlag`   BLOB, "
                      "PRIMARY KEY(`XmppId`))";

    try {
        st::sqlite::statement query(*_pSqlDb, sql);
        return query.executeStep();
    }
    catch (const std::exception &e) {
        error_log("CREATE TABLE IM_Friend_List error {0}", e.what());
        return false;
    }
}


bool FriendListDao::bulkInsertFriends(const std::vector<st::entity::IMFriendList> &friends) {
    if (!_pSqlDb) {
        return false;
    }

    std::string sql = "INSERT OR REPLACE INTO IM_Friend_List ('UserId', 'XmppId', 'Version') VALUES (?, ?, ?);";

    st::sqlite::statement query(*_pSqlDb, sql);
    try {
        //_pSqlDb->exec("BEGIN;");
        for (const st::entity::IMFriendList &friendinfo : friends) {
            query.bind(1, friendinfo.UserId);
            query.bind(2, friendinfo.XmppId);
            query.bind(3, friendinfo.Version);

            query.executeStep();
            query.resetBindings();
            //query.resetBindings();
        }
        query.clearBindings();
        // query.clearBindings();
        //_pSqlDb->exec("COMMIT;");
        return true;
    } catch (std::exception &e) {
        // query.clearBindings();
        //_pSqlDb->exec("ROLLBACK;");

        warn_log("bulkInsertFriends failed! {0}", e.what());

        return false;
    }
}

bool FriendListDao::insertFriend(st::entity::IMFriendList imfriend) {

    if (!_pSqlDb) {
        return false;
    }

    std::string sql = "INSERT OR REPLACE INTO IM_Friend_List (`UserId`, `XmppId`, `Version`) VALUES (?, ?, ?);";

    st::sqlite::statement query(*_pSqlDb, sql);
    try {
        query.bind(1, imfriend.UserId);
        query.bind(2, imfriend.XmppId);
        query.bind(3, imfriend.Version);

        return query.executeStep();
    } catch (std::exception &e) {
        return false;
    }
}

bool FriendListDao::deleteAllFriends() {
    if (!_pSqlDb) {
        return false;
    }

    std::string sql = "DELETE FROM IM_Friend_List;";

    st::sqlite::statement query(*_pSqlDb, sql);
    try {
        return query.executeStep();
    } catch (std::exception &e) {
        return false;
    }
}

bool FriendListDao::deleteFriendByXmppId(const std::string &xmppId) {

    if (!_pSqlDb) {
        return false;
    }

    std::string sql = "DELETE FROM IM_Friend_List WHERE `XmppId` = ?;";

    st::sqlite::statement query(*_pSqlDb, sql);
    try {
        query.bind(1, xmppId);
        return query.executeStep();
    } catch (std::exception &e) {
        return false;
    }
}

bool FriendListDao::getAllFriends(std::vector<st::entity::IMFriendList> &friends) {
    if (!_pSqlDb) {
        return false;
    }

    std::string sql = "SELECT `UserId`, `XmppId`, `Version` FROM `IM_Friend_List`;";

    st::sqlite::statement query(*_pSqlDb, sql);
    try {
        while (query.executeNext()) {
            st::entity::IMFriendList imfriend;
            imfriend.UserId = query.getColumn(0).getString();
            imfriend.XmppId = query.getColumn(1).getString();
            imfriend.Version = query.getColumn(2).getInt();

            friends.push_back(imfriend);
        }
        return true;
    } catch (std::exception &e) {
        return false;
    }
}
