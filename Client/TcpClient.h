#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include "MessageISO8583.h"

#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio/streambuf.hpp>

class TcpClient;
using ClientPtr = boost::shared_ptr<TcpClient>;

class TcpClient: public boost::enable_shared_from_this<TcpClient>, boost::noncopyable
{
public:		

public:
	explicit TcpClient(boost::asio::io_service & service, const Iso8583::Message& message, const std::string& clientName = {});
	virtual ~TcpClient();

	static ClientPtr create(boost::asio::io_service& service, const Iso8583::Message& message, const std::string& clientName = {});

	inline std::string fullName() const { return "Client " + mName; };

public:
	void start(const boost::asio::ip::tcp::endpoint& endpoint);
	void stop();
	
	void write();
	void read();

	inline bool isStarted() const { return mStarted; }
	inline boost::asio::ip::tcp::socket& socket() { return mSocket; }
	inline static unsigned short defaultPort() { return 2004; }	

private:
	void onConnection(const boost::system::error_code & error);
	void onWrite(const boost::system::error_code & error, size_t transferredBytes);
	void onRead(const boost::system::error_code & error, size_t transferredBytes);	
private:   
	
	void report(const std::string& message);

private:
	boost::asio::ip::tcp::socket   mSocket;
	std::string                    mName;		
	bool                           mStarted = false;	
	boost::asio::streambuf         mReadBuffer;
	boost::asio::streambuf         mWriteBuffer;

	const Iso8583::Message         mMessage;
};


#endif

