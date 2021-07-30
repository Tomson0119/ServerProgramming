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

		cout << "Connection Succeeded\n";

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

			if (clientSck.mReceiveBuffer == data)
			{
				cout << "Server received successfully\n";
			}
			else
			{
				
			}
		}
		cout << "Closing connection...\n";
	}
	catch (Exception& ex)
	{
		cout << ex.GetLog() << endl;
	}
}