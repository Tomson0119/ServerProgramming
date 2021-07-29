#include "../Net/common.h"
#include <iostream>

using namespace std;

constexpr int Server_port = 5505;

int main()
{
	try {
		Socket clientSck(SocketType::TCP);
		clientSck.Bind(EndPoint::Any);

		cout << "Connecting to Server...\n";
		
		// localhost(127.0.0.1)에 연결을 요청한다.
		clientSck.Connect(EndPoint("127.0.0.1", Server_port));

		cout << "Connection Succeeded: " << endl;
	}
	catch (Exception& ex)
	{
		cout << ex.GetLog() << endl;
	}
}