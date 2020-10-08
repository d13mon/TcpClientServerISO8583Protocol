#include "TcpClient.h"

#include <iostream>
#include <clocale>
#include <iomanip>
#include <cstring>
#include <fstream>

#include <boost/algorithm/string.hpp>

using namespace std;
using namespace boost;
using namespace boost::asio;
using namespace Iso8583;

class input_data_file_error : public std::exception {
public:
	input_data_file_error(const string& what = string("Input file parse error"))
		: std::exception(what.c_str())
	{}
};

int64_t toMillisecondsFromEpoch(const boost::posix_time::ptime& pt)
{
	using boost::posix_time::ptime;
	using namespace boost::gregorian;
	
	return  (pt - ptime(date(1970, Jan, 1))).total_milliseconds();	
}

std::string generateClientName()
{
	auto nowTime{ boost::posix_time::microsec_clock::local_time() };
	auto msCount = toMillisecondsFromEpoch(nowTime);
	stringstream ss;
	ss << msCount;		

	return ss.str();
}

Message createTestMessage()
{
	Message message("0800");
	message.setField(2, "4276200012341234");
	message.setField(6, "123456789012");
	message.setField(7, "0304172600");
	message.setField(11, "001205");	

	return message;
}

Message readMessageFromFile(const std::string & filename) {	
	
	ifstream file(filename);
	if (!file) {		
		throw input_data_file_error{ "Can't open the input file" };
	}

	string mti;
	std::getline(file, mti);
	if (mti.empty()){
        throw input_data_file_error{};
	}

	const int RECORDS_COUNT_IN_LINE_REQUIRED = 2;

	Message message(mti);

	string line;
	while (std::getline(file, line)) {
		vector<string> fields;

		boost::algorithm::split(fields, line, boost::is_any_of(", "));
		if (fields.size() != RECORDS_COUNT_IN_LINE_REQUIRED) {
			throw input_data_file_error{};
		}
		
		short fieldNumber;
		string fieldData;

		if (!(stringstream(fields[0]) >> fieldNumber)) {
			throw input_data_file_error{};
		}
		if (!(stringstream(fields[1]) >> fieldData)) {
			throw input_data_file_error{};
		}

		message.setField(fieldNumber, fieldData);
	}	

	return message;
}

int main(int argc, char* argv[])
{
	setlocale(LC_CTYPE, "rus");

	try {		
		const std::string inputFilename{ argc >= 2 ? argv[1] : "" };
		std::string clientName{ argc >= 3 ? argv[2] : "" };

		Message message;
		if (!inputFilename.empty()) {
			try {
               message = readMessageFromFile(inputFilename);
			}
			catch (const std::exception& ex){
				cerr << ex.what() << endl;
			}
		}
		
		if(message.isNull())
		{
			message = createTestMessage();
		}
#if 0
			cout << message;
#endif	

		if (clientName.empty())
		{
			clientName = generateClientName();
		}	

		io_service service;
		ip::tcp::endpoint endpoint(ip::address::from_string("127.0.0.1"), TcpClient::defaultPort());

		auto client = TcpClient::create(service, message, clientName);			

		client->start(endpoint);
		service.run();
	}
	catch (const std::exception& ex)
	{
		cerr << ex.what() << endl;
		return 1;
	}
}

