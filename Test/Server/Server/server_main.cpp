#include "../Net/common.h"
#include <iostream>

using namespace std;

constexpr int Server_port = 5505;

int main()
{
	try {
		Socket listenSck(SocketType::TCP);
		listenSck.Bind(EndPoint("0.0.0.0", Server_port));

	}
	catch (Exception& ex)
	{
		cout << ex.what() << endl;
	}
}