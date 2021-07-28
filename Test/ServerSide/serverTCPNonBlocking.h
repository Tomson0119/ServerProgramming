#pragma once

#include "../Network/common.h"
#include <iostream>
#include <thread>
#include <mutex>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <deque>
#include <csignal>


using namespace std;

// 컴파일러의 최적화를 벗어나는 volatile 변수로써 프로그램의 종료를 결정한다.
volatile bool loop = true;

void signal_handler(int signal)
{
	if (signal == SIGINT) {
		loop = false;
	}
}

void NonBlockingSocket()
{
	// ctrl + c 를 누르면 루프를 종료한다.
	std::signal(SIGINT, signal_handler);

	try {
		struct RemoteClient
		{
			Socket tcpConnection;	// Accept한 TCP 객체
		};
		unordered_map<RemoteClient*, shared_ptr<RemoteClient>> remoteClients;

		Socket listen(SocketType::TCP);
		listen.Bind(EndPoint("0.0.0.0", 5555));
		listen.SetNonBlocking(); // 논블록 소켓으로 세팅한다.
		listen.Listen();

		cout << "서버가 시작되었습니다.\n";
		cout << "CTRL + C 를 누르면 프로그램을 종료합니다.\n";

		// 리스닝 소켓과 연결 소켓 모두 I/O 가능 이벤크가 발생할 때까지 기다린다.
		
		// 소켓 핸들에 대해서 poll을 한다.
		vector<PollFD> readFds;
		// 어느 소켓이 어느 RemoteClient에 대한 것인지 가리킨다.
		vector<RemoteClient*> readFdsToRemoteClients;

		while (loop)
		{
			readFds.reserve(remoteClients.size() + 1);
			readFds.clear();
			readFdsToRemoteClients.reserve(remoteClients.size() + 1);
			readFdsToRemoteClients.clear();

			for (auto p : remoteClients)
			{
				PollFD item;
				item.mPollfd.events = POLLRDNORM;
				item.mPollfd.fd = p.second->tcpConnection.mHandle;
				item.mPollfd.revents = 0;
				readFds.push_back(item);
				readFdsToRemoteClients.push_back(p.first);
			}

			PollFD listenItem;
			listenItem.mPollfd.events = POLLRDNORM;
			listenItem.mPollfd.fd = listen.mHandle;
			listenItem.mPollfd.revents = 0;
			readFds.push_back(listenItem);

			// 100ms 동안 readFds의 pollFd 중 이벤트가 생성될 때까지 기다린다.
			// 수행 후 readFds의 이벤트가 발생한 pollfd의 revent는 0이 아닐 것이다.
			Poll(readFds.data(), (int)readFds.size(), 100);
			
			// readFds를 수색해서 필요한 것을 처리한다.
			int num = 0;
			for (auto fd : readFds)
			{
				if (fd.mPollfd.revents != 0)
				{
					if (num == readFds.size() - 1) // 리스닝 소켓
					{
						auto remoteClient = make_shared<RemoteClient>();

						// 클라이언트 연결이 들어온 상태이므로 그냥 호출만 해도 됨
						string ignore;
						listen.Accept(remoteClient->tcpConnection, ignore);
						remoteClient->tcpConnection.SetNonBlocking();

						// 이를 클라이언트 목록에 추가한다.
						remoteClients.insert({ remoteClient.get(), remoteClient });
						cout << "Client joined. There are " << remoteClients.size() << " connections.\n";
					}
					else {  // TCP 연결 소켓
						// 해당 인덱스의 클라이언트 객체를 빼온다.
						RemoteClient* remoteClient = readFdsToRemoteClients[num];

						int ec = remoteClient->tcpConnection.Receive();
						if (ec <= 0)
						{
							// 오류가 발생했으므로 해당 소켓은 종료
							remoteClient->tcpConnection.Close();
							remoteClients.erase(remoteClient);

							cout << "Client left. There are " << remoteClients.size() << " connections.\n";
						}
						else {
							// 받은 데이터를 그대로 송신한다.
							// 데이터 불일치의 문제는 생략
							remoteClient->tcpConnection.Send(remoteClient->tcpConnection.mReceiveBuffer, ec);
						}
					}
				}
				num++;
			}			
		}

		// 루프를 빠져나가면 모두 종료.
		listen.Close();
		remoteClients.clear();
	}
	catch (Exception& ex)
	{
		cout << "Exception! " << ex.ErrorLog() << '\n';
	}
}
