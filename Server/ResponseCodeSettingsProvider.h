#ifndef RESPONSECODESETTINGSPROVIDER_H
#define RESPONSECODESETTINGSPROVIDER_H

#include "OracleDbClient.h"

#include <unordered_map>
#include <memory>

using SettingsMap = std::unordered_map<std::string, std::string>;

class ResponseCodeSettingsProvider;
using RCSettingsProviderPtr = std::shared_ptr<ResponseCodeSettingsProvider>;

class ResponseCodeSettingsProvider : public OracleDbClient
	                               , public std::enable_shared_from_this<ResponseCodeSettingsProvider>{
public:
	ResponseCodeSettingsProvider(const std::string& username = {}, const std::string& password = {});
	virtual ~ResponseCodeSettingsProvider();

	static RCSettingsProviderPtr create(const std::string& username = {}, const std::string& password = {});
	   
	inline static const std::string defaultUsername() { return "C##Scott"; }
	inline static const std::string defaultPassword() { return "tiger"; }
	inline static const std::string defaultTableName() { return "response_code_settings"; }

	SettingsMap querySettings();
};



#endif

