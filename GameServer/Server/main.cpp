#include "common.h"
#include "Server.h"

#include <iostream>

using namespace std;

const short SERVER_PORT = 5505;

int main()
{
	try {
		Server server(Protocol::TCP, EndPoint::Any(SERVER_PORT));
		server.Run();
		return 0;
	}
	catch (NetException& ex)
	{
		cout << ex.what() << endl;
		return -1;
	}
}