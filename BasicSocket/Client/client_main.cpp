#include "../Network/common.h"
#include <iostream>
#include <fstream>

using namespace std;

constexpr int Server_port = 5505;

void BlockingTCPSocket();

int main()
{
	BlockingTCPSocket();
}

void BlockingTCPSocket()
{
	try {
		Socket clientSck(SocketType::TCP);
		clientSck.Bind(EndPoint::Any);

		cout << "Connecting to Server...\n";

		clientSck.Connect(EndPoint("127.0.0.1", Server_port));

		cout << "Sending data...\n";
		string data;
		int recv_len = 0;
		while (true)
		{
			data.clear();
			getline(cin, data);

			// 데이터를 문자열 끝과 함께 보낸다.
			clientSck.Send(data.c_str(), data.size());

			// 서버로부터 응답 데이터를 받는다.
			// 데이터 끝은 의도치 않은 문자일 수 있으므로 비워버린다.
			int recv_len = clientSck.Receive();
			clientSck.mReceiveBuffer[recv_len] = 0;

			if (clientSck.mReceiveBuffer != data)
			{
				cout << "Server did not received successfully\n";
				break;
			}
		}
		cout << "Closing connection...\n";
	}
	catch (Exception& ex)
	{
		cout << ex.GetLog() << endl;
	}
}