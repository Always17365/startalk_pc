//
// Created by cc on 2019/10/15.
//

#include "MedalListDao.h"
#include "Util/Log.h"

MedalListDao::MedalListDao(st::sqlite::database *sqlDb)
    :DaoInterface(sqlDb, "IM_Medal_List")
{
}

bool MedalListDao::creatTable() {
    if (!_pSqlDb) {
        return false;
    }

    std::string sql = "CREATE TABLE IF NOT EXISTS IM_Medal_List( "
                      " medalId               INTEGER PRIMARY KEY, "
                      " medalName             TEXT, "
                      " obtainCondition       TEXT, "
                      " smallIcon             TEXT, "
                      " bigLightIcon          TEXT, "
                      " bigGrayIcon           TEXT, "
                      " bigLockIcon           BLOB, "
                      " status                INTEGER "
                      " );";

    try {
        st::sqlite::statement query(*_pSqlDb, sql);
        return query.executeStep();
    }
    catch (const std::exception &e) {
        error_log("CREATE TABLE  IM_Medal_List error {0}", e.what());
    }
    return false;
}


void MedalListDao::insertMedalList(const std::vector<st::entity::ImMedalList> &medals) {
    if (!_pSqlDb) {
        return;
    }

    std::string sql = "INSERT INTO IM_Medal_List(`medalId`, `medalName`, `obtainCondition`, `smallIcon`, `bigLightIcon`, "
        " `bigGrayIcon`, `bigLockIcon`, `status` ) VALUES(?,?,?,?,?,?,?,?) "
        "ON CONFLICT(medalId) DO "
        "update set medalName = ?, obtainCondition = ?, `smallIcon` = ?, `bigLightIcon` = ? "
        ", `bigGrayIcon` = ?, `bigLockIcon` = ?, `status` = ? where medalId = ? ";
    st::sqlite::statement query(*_pSqlDb, sql);

    try {
        _pSqlDb->exec("begin immediate;");
        for(const auto & m : medals)
        {
            query.bind(1, m.medalId);
            query.bind(2, m.medalName);
            query.bind(3, m.obtainCondition);
            query.bind(4, m.smallIcon);
            query.bind(5, m.bigLightIcon);
            query.bind(6, m.bigGrayIcon);
            query.bind(7, m.bigLockIcon);
            query.bind(8, m.status);
            query.bind(9, m.medalName);
            query.bind(10,m.obtainCondition);
            query.bind(11,m.smallIcon);
            query.bind(12,m.bigLightIcon);
            query.bind(13,m.bigGrayIcon);
            query.bind(14,m.bigLockIcon);
            query.bind(15,m.status);
            query.bind(16, m.medalId);

            query.executeStep();
            query.resetBindings();
        }
        query.clearBindings();
        _pSqlDb->exec("commit transaction;");
    }
    catch (const std::exception& e)
    {
        _pSqlDb->exec("rollback transaction;");
        error_log("exception :{0}", e.what());
    }
}

//
void MedalListDao::getMedalList(std::vector<st::entity::ImMedalList> &medals) {
    std::string sql =
            "select medalId, medalName, obtainCondition, smallIcon, bigLightIcon, bigLockIcon, status "
            "from IM_Medal_List where status <> 0";

    st::sqlite::statement query(*_pSqlDb, sql);

    while (query.executeNext())
    {
        st::entity::ImMedalList medal;
        medal.medalId = query.getColumn(0).getInt();
        medal.medalName = query.getColumn(1).getString();
        medal.obtainCondition = query.getColumn(2).getString();
        medal.smallIcon = query.getColumn(3).getString();
        medal.bigLightIcon = query.getColumn(4).getString();
        medal.bigLockIcon = query.getColumn(5).getString();
        medal.status = query.getColumn(6).getInt();
        medals.push_back(medal);
    }
}
