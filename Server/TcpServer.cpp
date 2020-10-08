#include "TcpServer.h"

#include <cassert>
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <vector>

#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
using namespace boost::asio;
using namespace boost::placeholders;
using namespace Iso8583;

TcpServer::TcpServer(boost::asio::io_service& service)
	: mSocket(service)	
{
	auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	mRandomizer.seed(static_cast<unsigned>(seed));
	
	if (readResponseCodeSettings())
		report("Read response code settings success.");

	report("Waiting for connection...");	
}

TcpServer::~TcpServer()
{
	stop();
}

ServerPtr TcpServer::create(boost::asio::io_service& service)
{
	return ServerPtr(new TcpServer(service));
}

void TcpServer::start()
{
	report("Waiting for new connections...");

	mStarted = true;
	read();	
}

void TcpServer::stop()
{
	if (!mStarted)
		return;

	mStarted = false;
	mSocket.close();
}

void TcpServer::read()
{
	if (!mStarted)
		return;

	async_read_until(mSocket, mReadBuffer, '\n', boost::bind(&TcpServer::onRead, shared_from_this(), _1, _2));
}

void TcpServer::onRead(const boost::system::error_code & error, size_t transferredBytes)
{
	if (!error) {

		std::istream in(&mReadBuffer);		

		vbytes bytes{ std::istreambuf_iterator<char>{in} ,{} };
#if 0
        cout <<  "READ = " << mReadBuffer.size() << endl;	
		std::cout << "message = " << bytes.size() << " bytes" << std::hex << std::setfill('0') << endl;
		for (byte_ b : bytes) std::cout << std::setw(2) << int(b) << ' ';
		cout << endl;
#endif
		report("Message from client received:\n");

		try {
			mMessage = Message::fromBytes(bytes);			
			cout << mMessage << endl;
		}	
		catch (const std::exception& ex) {
			reportError(ex.what());
		}		

		processClientMessage();

		write();
	}
	else {
		report(error.message());
	}

	stop();
}

void TcpServer::processClientMessage()
{
	if (checkPAN()) {
		checkCardholderBilling();
	}	

	responseCodeMappings();

	mMessage.setMTI("0810");
	mMessage.setField(39, mResponseCode);

#if 0
	cout << mMessage;
#endif
}

bool TcpServer::checkPAN()
{
	const std::vector<std::string> errorCodes 
		= {"00", "00", "00", "14", "15", "36", "41", "43", "54", "56", "57", "00", "00"};

	if (auto field = mMessage.field(2)) {		
		//NOTE: Some field checking here
		//auto pan = field->toString();		
	   
		mResponseCode = generateResponseCode(errorCodes);
	}	

	return !isErrorResponseCode();
}

bool TcpServer::checkCardholderBilling()
{
	const std::vector<std::string> errorCodes = { "00", "00", "39", "58", "64", "00" };

	if (auto field = mMessage.field(6)) {
		//NOTE: Some field checking here
		//auto cardholderBilling = field->toString();		

		mResponseCode = generateResponseCode(errorCodes);
	}

	return !isErrorResponseCode();
}

void TcpServer::responseCodeMappings()
{
	if (mRCSettings.empty())
		return;

	if (mRCSettings.count(mResponseCode)) {
		mResponseCode = mRCSettings[mResponseCode];
	}
}

std::string TcpServer::generateResponseCode(const RCList& rcList)
{
	if (rcList.empty())
		return string{};

	size_t maxValue = rcList.size()-1;

	uniform_int_distribution<size_t> dist(0, maxValue);
	auto index = dist(mRandomizer);

	return rcList[index];
}

bool TcpServer::readResponseCodeSettings()
{
	try {
		auto settingsProvider = ResponseCodeSettingsProvider::create();
		mRCSettings = settingsProvider->querySettings();
#if 0
		for (const auto& s : mRCSettings) {
			cout << " Mapping: " << s.first << " -> " << s.second << endl;
		}
#endif
	}
	catch (const std::exception & ex) {
		report(ex.what());
		return false;
	}

	return true;
}

void TcpServer::write()
{
	if (!mStarted)
		return;	

	std::ostream out(&mWriteBuffer);	
	
	auto bytes = mMessage.toBytes();
	out.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
	out << '\n';		

	async_write(mSocket, mWriteBuffer, boost::bind(&TcpServer::onWrite, shared_from_this(), _1, _2));
}

void TcpServer::onWrite(const boost::system::error_code & error, size_t transferredBytes)
{
	if (!error) {
		read();
	}
	else {
		stop();
		report(error.message());		
	}
}

void TcpServer::reportError(const std::string& what)
{	
	//NOTE: Code 06 for server errors
	mResponseCode = "06";

	report("Error: " + what);
}

void TcpServer::report(const std::string& message)
{	
	cout << name() << ": " << message << endl;	
}

bool TcpServer::isErrorResponseCode() const
{
	return mResponseCode != "00";
}

