#include "OracleDbClient.h"

#include <iostream>
#include <sstream>

using namespace oracle::occi;
using namespace std;


oracle_db_error::oracle_db_error(const std::string& what)
	: std::exception(what.c_str())
{

}

OracleDbClient::OracleDbClient(const std::string& username, const std::string& password, const std::string& connectionName /*= {}*/)
	: mUsername(username)
	, mPassword(password)
	, mConnectionName(connectionName)
{
}

OracleDbClient::~OracleDbClient()
{
	if (mEnvironment)
	{
		if (mConnection) {
	        mEnvironment->terminateConnection(mConnection);
		}	
		Environment::terminateEnvironment(mEnvironment);
	}	
}

std::string OracleDbClient::getErrorMessage(const SQLException& ex)
{
	stringstream ss;
	ss << ex.getErrorCode() << " " << ex.getMessage();

	return ss.str();
}

bool OracleDbClient::connect()
{
	try
	{		
		mEnvironment = Environment::createEnvironment();
		if (!mEnvironment) 
			return false;
		
		mConnection = mEnvironment->createConnection(mUsername, mPassword, mConnectionName);
		if (!mConnection)
			return false;		
	}
	catch (const SQLException& ex)
	{
		throw oracle_db_error(getErrorMessage(ex));		
	}

	return true;
}

ResultSet* OracleDbClient::execute(const std::string& query)
{
	if (!mConnection)
		return nullptr;

	try
	{
		mCurrentQueryStatement = mConnection->createStatement(query);
		if (!mCurrentQueryStatement)
			return nullptr;
	
		return mCurrentQueryStatement->executeQuery();		
	}
	catch (const SQLException& ex)
	{
		throw oracle_db_error(getErrorMessage(ex));		
	}

	return nullptr;
}



