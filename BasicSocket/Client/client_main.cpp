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

			// �����͸� ���ڿ� ���� �Բ� ������.
			clientSck.Send(data.c_str(), data.size());

			// �����κ��� ���� �����͸� �޴´�.
			// ������ ���� �ǵ�ġ ���� ������ �� �����Ƿ� ���������.
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