//
// Created by 王超 on 2020/6/8.
//

#include "ProcessExceptDao.h"

ProcessExceptDao::ProcessExceptDao(st::sqlite::database *sqlDb)
    : DaoInterface(sqlDb, "IM_PROCESS_EXCEPT") {
}

bool ProcessExceptDao::creatTable() {
    if(!_pSqlDb) return false;
    //
    std::string sql = "CREATE TABLE IM_PROCESS_EXCEPT ( "
                      "`EXCEPT_TYPE` TEXT, "
                      "`EXCEPT_NUM`  REAL, "
                      "`EXCEPT_TIME` INTEGER, "
                      "`EXCEPT_STACK` TEXT) ";

    st::sqlite::statement query(*_pSqlDb, sql);

    return query.executeStep();
}

void ProcessExceptDao::addExceptCpu(double cpu, long long time, const std::string &stack) {
    if(!_pSqlDb) return;

    std::string sql = "INSERT INTO IM_PROCESS_EXCEPT (EXCEPT_TYPE, EXCEPT_NUM, EXCEPT_TIME, EXCEPT_STACK) VALUES "
                      "(\"CPU\", ?, ?, ?)";
    st::sqlite::statement query(*_pSqlDb, sql);
    query.bind(1, (long)cpu);
    query.bind(2, time);
    query.bind(3, stack);

    query.executeStep();
}
