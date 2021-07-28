#pragma once

#include "../Network/common.h"
#include <iostream>

using namespace std;

void BlockingSocketExample()
{
	try {
		Socket listening(SocketType::TCP, true);
		// 0.0.0.0�� ���� ����� ��� ��Ʈ��ũ�� ��ȣ�� ����϶�� �ǹ��̴�.
		listening.Bind(EndPoint("0.0.0.0", 5959));

		cout << "Listening connection...\n";
		listening.Listen();

		Socket connection;
		string ignore;
		listening.Accept(connection, ignore);

		EndPoint client = connection.GetPeerAddr();
		cout << "Socket from " << client.ToString() << " is accepted.\n";

		cout << "Receiving data..\n";
		while (true)
		{
			int len = connection.Receive();

			if (len == 0) {
				cout << "Connection closing...\n";
				break;
			}
			else if (len < 0) {
				cout << "Connect lost : " << GetLastErrorMessage() << '\n';
				break;
			}

			string recvData;
			// ������ ���ڰ� �ǵ�ġ ���� ������ �� �����Ƿ� ������´�.
			connection.mReceiveBuffer[len] = NULL;
			recvData.assign(connection.mReceiveBuffer, len);

			cout << "Received : " << recvData << '\n';

			// ACK������ν� �ٽ� �����ش�.			
			connection.Send(recvData);
		}
	}
	catch (Exception& ex)
	{
		cout << ex.ErrorLog() << '\n';
	}
}