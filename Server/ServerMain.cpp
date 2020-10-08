#include "TcpServer.h"

#include <iostream>
#include <clocale>

#include <boost/bind.hpp>

using namespace std;
using namespace boost::placeholders;
using namespace boost::asio;

io_service gService;
ip::tcp::acceptor acceptor{ gService, ip::tcp::endpoint(ip::tcp::v4(), TcpServer::defaultPort()) };

void handleClient(ServerPtr server, const boost::system::error_code & error) {
	server->start();
	ServerPtr newServerInstance = TcpServer::create(gService);
	acceptor.async_accept(newServerInstance->socket(), boost::bind(handleClient, newServerInstance, _1));
}

int main(int argc, char* argv[])
{ 
	setlocale(LC_CTYPE, "rus");

	try { 	
	   auto serverInstance = TcpServer::create(gService);
	   acceptor.async_accept(serverInstance->socket(), boost::bind(handleClient, serverInstance, _1));
	   gService.run();
	}	
	catch (std::exception& ex)
	{
		cerr << ex.what() << endl;
		return 1;
	}

	return 0;
}

