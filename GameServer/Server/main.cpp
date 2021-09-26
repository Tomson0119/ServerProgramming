#include "common.h"
#include <iostream>

using namespace std;

const short SERVER_PORT = 5505;

int main()
{
	try {
		Socket listenSck(Protocol::TCP);
		EndPoint serverEp = EndPoint::Any(SERVER_PORT);
		listenSck.Bind(serverEp);
		listenSck.Listen();
		Socket clientSck = listenSck.Accept(serverEp);
		
		cout << "Client has accepted\n";
		while (1)
		{

		}
		

		return 0;
	}
	catch (NetException& ex)
	{
		cout << ex.what() << endl;
		return -1;
	}
}