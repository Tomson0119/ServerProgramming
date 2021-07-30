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
		
		// localhost(127.0.0.1)�� ������ ��û�Ѵ�.
		clientSck.Connect(EndPoint("127.0.0.1", Server_port));

		cout << "Connection Succeeded\n";

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