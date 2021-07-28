#pragma once

#include "../Network/common.h"
#include <iostream>

using namespace std;

void BlockingSocketExample()
{
	try {
		Socket listening(SocketType::TCP, true);
		// 0.0.0.0은 로컬 기기의 모든 네트워크의 신호에 대기하라는 의미이다.
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
			// 마지막 문자가 의도치 않은 문자일 수 있으므로 비워놓는다.
			connection.mReceiveBuffer[len] = NULL;
			recvData.assign(connection.mReceiveBuffer, len);

			cout << "Received : " << recvData << '\n';

			// ACK기능으로써 다시 보내준다.			
			connection.Send(recvData);
		}
	}
	catch (Exception& ex)
	{
		cout << ex.ErrorLog() << '\n';
	}
}