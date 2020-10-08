#include "ResponseCodeSettingsProvider.h"

#include <iostream>

using namespace std;
using namespace oracle::occi;

ResponseCodeSettingsProvider::ResponseCodeSettingsProvider(const std::string& username, const std::string& password)
	: OracleDbClient(username, password)
{
	if(!connect())
		throw oracle_db_error("Can't connect to database");
}

ResponseCodeSettingsProvider::~ResponseCodeSettingsProvider()
{
}

RCSettingsProviderPtr ResponseCodeSettingsProvider::create()
{
	return RCSettingsProviderPtr(new ResponseCodeSettingsProvider(defaultUsername(), defaultPassword()));
}

SettingsMap ResponseCodeSettingsProvider::querySettings()
{
	SettingsMap settings;	

	try
	{
		const std::string query = "select * from " + defaultTableName();
		if (auto resultSet = execute(query)) {
			while (resultSet->next())
			{
				std::string inputCode = resultSet->getString(2);
				std::string resultCode = resultSet->getString(3);
#if 0
				cout << " Mapping: " << inputCode << " -> " << resultCode  << endl;
#endif
				settings[inputCode] = resultCode;				
			}

			mCurrentQueryStatement->closeResultSet(resultSet);
		}

		mConnection->terminateStatement(mCurrentQueryStatement);
		mCurrentQueryStatement = nullptr;
	}
	catch (const SQLException& ex)
	{
		throw oracle_db_error(getErrorMessage(ex));
	}

	return settings;
}

