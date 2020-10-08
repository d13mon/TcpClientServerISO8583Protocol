#ifndef TCPSERVER_H
#define TCPSERVER_H

#include "MessageISO8583.h"

#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/streambuf.hpp>

#include "ResponseCodeSettingsProvider.h"

#include <string>
#include <random>

using RCList = std::vector<std::string>;

class TcpServer;
using ServerPtr = boost::shared_ptr<TcpServer>;

class TcpServer : public boost::enable_shared_from_this<TcpServer>, boost::noncopyable
{	
public: 
	explicit TcpServer(boost::asio::io_service& service);
	virtual ~TcpServer();

	static ServerPtr create(boost::asio::io_service& service);	

    inline const std::string defaultName() { return "Server"; }
	inline const std::string name() { return defaultName(); }
public:

	void start();
	void stop();
	
	void read();
	void write();

	inline bool isStarted() const { return mStarted; }
	inline static unsigned short defaultPort() { return 2004; }
	inline boost::asio::ip::tcp::socket& socket() { return mSocket; }

private:
	void onWrite(const boost::system::error_code & error, size_t transferredBytes);
	void onRead(const boost::system::error_code & error, size_t transferredBytes);	

private:   
	void reportError(const std::string& what);
	void report(const std::string& message);

	bool isErrorResponseCode() const;
	
	void processClientMessage();

	bool checkPAN();
	bool checkCardholderBilling();
	void responseCodeMappings();

	std::string generateResponseCode(const RCList& rcList);

	bool readResponseCodeSettings();

private:

	boost::asio::ip::tcp::socket   mSocket;
	bool                           mStarted = false;
	boost::asio::streambuf         mReadBuffer;
	boost::asio::streambuf         mWriteBuffer;

	Iso8583::Message               mMessage;
	std::string                    mResponseCode = "00";

	SettingsMap                    mRCSettings;

	//NOTE: Used to generate Response codes (TEST feature)
	std::mt19937                   mRandomizer;
};


#endif

