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
		Socket listenSck(SocketType::TCP);
		// 0.0.0.0은 로컬 기기의 모든 네트워크의 주소를 가리킨다.
		listenSck.Bind(EndPoint("0.0.0.0", Server_port));

		cout << "Listening to Clients...\n";
		listenSck.Listen();

		Socket connSck = listenSck.Accept();
		cout << "Client joined - " << connSck.GetPeerAddr().ToString() << '\n';

		cout << "Receiving data...\n";
		while (true)
		{
			// 데이터를 클라이언트로부터 받는다.
			// 데이터 끝은 의도치 않은 문자일 수 있으므로 비워버린다.
			int recv_len = connSck.Receive();
			connSck.mReceiveBuffer[recv_len] = 0;

			cout << "Data >> " << connSck.mReceiveBuffer << '\n';

			// 클라이언트에게 그대로 돌려준다.
			connSck.Send(connSck.mReceiveBuffer, recv_len + 1);
		}
		cout << "Closing server...\n";
	}
	catch (Exception& ex)
	{
		cout << ex.GetLog() << endl;
	}
}