#pragma once

#include "../Network/common.h"
#include <iostream>

using namespace std;

void BlockingSocketExample()
{
	try {
		Socket socket(SocketType::TCP, true);
		socket.Bind(EndPoint::Any);
		cout << "Connecting to Server...\n";

		// localhost(127.0.0.1)에 연결을 시도한다.
		socket.Connect(EndPoint("127.0.0.1", 5959));

		EndPoint server = socket.GetPeerAddr();
		cout << "Connection accepted from " << server.ToString() << '\n';

		cout << "Sending data...\n";
		while (true) {
			string data;
			getline(cin, data);
			socket.Send(data);

			int len = socket.Receive();

			// 마지막 문자가 의도치 않은 문자일 수 있으므로 비워놓는다.
			socket.mReceiveBuffer[len] = NULL;
			cout << "Server Message : " << socket.mReceiveBuffer << '\n';
			if (data.size() != len)
			{
				cout << "Closing socket\n";
				break;
			}
		}
	}
	catch (Exception& error)
	{
		cout << error.ErrorLog() << '\n';
	}
}