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

		// localhost(127.0.0.1)�� ������ �õ��Ѵ�.
		int result = socket.Connect(EndPoint("127.0.0.1", 5555));
		if (result == 0)
		{
			cout << "���� ����" << endl;
		}
		else if (WSAGetLastError() == WSAEWOULDBLOCK)
		{
			cout << "���� ��" << endl;

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
					cout << "���� ���� ��" << endl;
				}
				else
				{
					cout << "���� ����" << endl;
					break;
				}
			}
		}
		else
		{
			cout << "���� ����" << endl;
		}
	}
	catch (Exception& error)
	{
		cout << error.ErrorLog() << '\n';
	}
}