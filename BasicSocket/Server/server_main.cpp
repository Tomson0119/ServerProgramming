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
		// 0.0.0.0�� ���� ����� ��� ��Ʈ��ũ�� �ּҸ� ����Ų��.
		listenSck.Bind(EndPoint("0.0.0.0", Server_port));

		cout << "Listening to Clients...\n";
		listenSck.Listen();

		Socket connSck = listenSck.Accept();
		cout << "Client joined - " << connSck.GetPeerAddr().ToString() << '\n';

		cout << "Receiving data...\n";
		while (true)
		{
			// �����͸� Ŭ���̾�Ʈ�κ��� �޴´�.
			// ������ ���� �ǵ�ġ ���� ������ �� �����Ƿ� ���������.
			int recv_len = connSck.Receive();
			connSck.mReceiveBuffer[recv_len] = 0;

			cout << "Data >> " << connSck.mReceiveBuffer << '\n';

			// Ŭ���̾�Ʈ���� �״�� �����ش�.
			connSck.Send(connSck.mReceiveBuffer, recv_len + 1);
		}
		cout << "Closing server...\n";
	}
	catch (Exception& ex)
	{
		cout << ex.GetLog() << endl;
	}
}