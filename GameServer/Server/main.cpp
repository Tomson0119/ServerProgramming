#include "common.h"
#include "IOCPServer.h"

#include <iostream>

int main()
{
	try {
		IOCPServer server(EndPoint::Any(SERVER_PORT));
		server.Run();
		return 0;
	}
	catch (NetException& ex)
	{
		std::cout << ex.what() << std::endl;
		return -1;
	}
}