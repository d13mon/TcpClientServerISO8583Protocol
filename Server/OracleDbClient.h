#ifndef ORACLEDBCLIENT_H
#define ORACLEDBCLIENT_H

#include <string>
#include <occi.h>

class oracle_db_error : public std::exception {
public:
	oracle_db_error(const std::string& what);
};

class OracleDbClient{
public:
	OracleDbClient(const std::string& username, const std::string& password, const std::string& connectionName = {});
	virtual ~OracleDbClient();

	bool connect();		

private:
	OracleDbClient(const OracleDbClient&) = delete;
	OracleDbClient& operator=(const OracleDbClient&) = delete;

protected:
	oracle::occi::ResultSet* execute(const std::string& query);
	static std::string getErrorMessage(const oracle::occi::SQLException& ex);

protected:	
	std::string                mUsername;
	std::string                mPassword;
	std::string                mConnectionName;
	
	oracle::occi::Environment* mEnvironment = nullptr;
	oracle::occi::Connection*  mConnection = nullptr;
	oracle::occi::Statement*   mCurrentQueryStatement = nullptr;
};


#endif 

