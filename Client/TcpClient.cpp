#include "TcpClient.h"

#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <cassert>

#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
using namespace boost::asio;
using namespace boost::placeholders;
using namespace Iso8583;

TcpClient::TcpClient(io_service & service, const Iso8583::Message& message, const std::string& clientName)
	: mSocket(service)
	, mName(clientName)	
	, mMessage(message)
{	
}

TcpClient::~TcpClient()
{
	stop();
}

ClientPtr TcpClient::create(io_service& service, const Iso8583::Message& message, const std::string& clientName) {

	return ClientPtr(new TcpClient(service, message, clientName));
}

void TcpClient::start(const ip::tcp::endpoint& endpoint)
{
	mStarted = true;
	mSocket.async_connect(endpoint, boost::bind(&TcpClient::onConnection, shared_from_this(), _1));
}

void TcpClient::stop()
{
	if (!mStarted)
		return;

	mStarted = false;
	mSocket.close();
}

void TcpClient::onConnection(const boost::system::error_code & error)
{
	if (!error) {
		write();
	}
	else {
		stop();
		report(error.message());		
	}
}

void TcpClient::write()
{
	if (!mStarted)
		return;	
	
	std::ostream os(&mWriteBuffer);		

	auto bytes = mMessage.toBytes();

#if 0
	cout << "Client: write: " << bytes.size() << " bytes\n";
#endif

	os.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
	os << '\n';

	async_write(mSocket, mWriteBuffer, boost::bind(&TcpClient::onWrite, shared_from_this(), _1, _2));
}

void TcpClient::onWrite(const boost::system::error_code & error, size_t transferredBytes)
{
	if (!error) {
		read();
	}
	else {
		stop();
		report(error.message());		
	}
}

void TcpClient::read()
{
	if (!mStarted)
		return;	

	async_read_until(mSocket, mReadBuffer, '\n', boost::bind(&TcpClient::onRead, shared_from_this(), _1, _2));
}

void TcpClient::onRead(const boost::system::error_code & error, size_t transferredBytes)
{
	if (!error) {

		std::istream in(&mReadBuffer);

		vbytes bytes{ std::istreambuf_iterator<char>{in} ,{} };
		report("Server replied:\n");	

		try {
			auto message = Message::fromBytes(bytes);

			cout << message;
		}
		catch (const std::exception& ex) {
			report(ex.what());
		}			
	}
	else {
		report(error.message());
		cout << &mReadBuffer << endl;
	}

	stop();
}

void TcpClient::report(const std::string& message)
{
	cout << fullName() << ": " << message << endl;   
}



