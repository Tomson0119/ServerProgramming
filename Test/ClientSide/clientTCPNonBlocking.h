#pragma once

#include "../Network/common.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace std;
using namespace std::chrono_literals;

void NonBlockingSocketExample()
{
	try {
		Socket socket(SocketType::TCP);
		socket.Bind(EndPoint::Any);
		socket.SetNonBlocking();
		cout << "Connecting to Server...\n";

		// localhost(127.0.0.1)에 연결을 시도한다.
		int result = socket.Connect(EndPoint("127.0.0.1", 5555));
		if (result == 0)
		{
			cout << "연결 성공" << endl;
		}
		else if (WSAGetLastError() == WSAEWOULDBLOCK)
		{
			cout << "연결 중" << endl;

			while (true) {
				vector<char> empty;
				int result = socket.Send(empty.data(), 0);
				if (result == 0)
				{
					string data = "Hello World.";
					socket.Send(data);
					int len = socket.Receive();
					cout << socket.mReceiveBuffer << endl;
					this_thread::sleep_for(1s);
				}
				else if (result == WSAENOTCONN)
				{
					cout << "연결 진행 중" << endl;
				}
				else
				{
					cout << "연결 실패" << endl;
					break;
				}
			}
		}
		else
		{
			cout << "연결 실패" << endl;
		}
	}
	catch (Exception& error)
	{
		cout << error.ErrorLog() << '\n';
	}
}