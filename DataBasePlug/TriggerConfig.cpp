//
// Created by lihaibin on 2019-07-08.
//
#include "TriggerConfig.h"

TriggerConfig::TriggerConfig(st::sqlite::database *sqlDb) :
        _sqlDb(sqlDb) {

}

/**
 * update immessage readtag字段后 session未读数trigger
 * @return
 */
bool TriggerConfig::createUnreadUpdateTrigger() {
    if (!_sqlDb) {
        return false;
    }
    std::string sql = "CREATE TRIGGER if not exists sessionlist_unread_update "
                      "after update of ReadedTag on IM_Message "
                      "for each row begin update IM_SessionList set UnreadCount = "
                      "case when (new.ReadedTag & 2) =2 and old.ReadedTag & 2 <>2 "
                      "then (case when UnreadCount >0 then (unreadcount -1) else 0 end ) "
                      "when (new.ReadedTag & 2) <>2 and old.ReadedTag & 2 =2 "
                      "then (UnreadCount + 1) "
                      "else UnreadCount "
                      "end "
                      "where XmppId = new.XmppId and RealJid = new.realjid and new.'from' <> (select value from IM_Cache_Data where key ='USER_ID' and type = 6); end";
    st::sqlite::statement query(*_sqlDb, sql);
    return query.executeStep();
}

/**
 * insert immessage session未读数trigger
 * @return
 */
bool TriggerConfig::createUnreadInserttrigger() {
    if (!_sqlDb) {
        return false;
    }
    std::string sql = "CREATE TRIGGER if not exists  sessionlist_unread_insert "
                      "after insert on IM_Message "
                      "for each row begin update IM_SessionList set UnreadCount = "
                      "case when (new.ReadedTag & 2 )<>2 "
                      "then (UnreadCount+1) "
                      "else UnreadCount "
                      "end "
                      "where XmppId = new.XmppId and RealJid = new.realjid and new.'from' <> (select value from IM_Cache_Data where key ='USER_ID' and type = 6); end";
    st::sqlite::statement query(*_sqlDb, sql);
    return query.executeStep();
}

//
void TriggerConfig::modifyUnreadCountTrigger() {
    if (!_sqlDb) {
        return;
    }
    // delete
    {
        std::string updateSql = "DROP TRIGGER sessionlist_unread_update";
        std::string insertSql = "DROP TRIGGER sessionlist_unread_insert";

        st::sqlite::statement query_update(*_sqlDb, updateSql);
        bool ret = query_update.executeStep();
        st::sqlite::statement query_insert(*_sqlDb, insertSql);
        ret = query_insert.executeStep();
    }
    // new update
    {
        std::string sql = "CREATE TRIGGER IF NOT EXISTS sessionlist_unread_update "
                          "after update of ReadedTag on IM_Message "
                          "for each row begin update IM_SessionList set UnreadCount = "
                          "case when (new.ReadedTag & 2) =2 and old.ReadedTag & 2 <>2 "
                          "then (case when UnreadCount >0 then (unreadcount -1) else 0 end ) "
                          "when (new.ReadedTag & 2) <>2 and old.ReadedTag & 2 =2 "
                          "then (UnreadCount + 1) "
                          "else UnreadCount "
                          "end "
                          "where XmppId = new.XmppId and RealJid = new.realjid "
                          "and new.`Direction` = 0; end";
        st::sqlite::statement query(*_sqlDb, sql);
        query.executeStep();
    }

    // new insert
    {
        std::string sql = "CREATE TRIGGER if not exists  sessionlist_unread_insert "
                          "after insert on IM_Message "
                          "for each row begin update IM_SessionList set UnreadCount = "
                          "case when (new.ReadedTag & 2 )<>2 "
                          "then (UnreadCount+1) "
                          "else UnreadCount "
                          "end "
                          "where XmppId = new.XmppId and RealJid = new.realjid "
                          "and new.`Direction` = 0; end";
        st::sqlite::statement query(*_sqlDb, sql);
        query.executeStep();
    }
}
