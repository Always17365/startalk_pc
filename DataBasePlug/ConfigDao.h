#ifndef _CONFIGDAO_H_
#define _CONFIGDAO_H_

#include "DaoInterface.h"
#include "entity/im_config.h"
#include <vector>

class ConfigDao : public DaoInterface
{
public:
	explicit ConfigDao(st::sqlite::database *sqlDb);
	bool creatTable() override;

public:
    bool getConfigVersion(int& version);
    bool bulkInsertConfig(const std::vector<st::entity::ImConfig>& config);
    bool bulkRemoveConfig(const std::map<std::string, std::string>& mapConf);
	bool insertConfig(const std::string& key, const std::string& subKey, const std::string& val);
	bool getConfig(const std::string& key, const std::string& subKey, std::string& val);
	bool getConfig(const std::string& key, std::map<std::string, std::string>& mapConf);
    bool getAllConfig(std::vector<st::entity::ImConfig>& configs);

};

#endif//_CONFIGDAO_H_