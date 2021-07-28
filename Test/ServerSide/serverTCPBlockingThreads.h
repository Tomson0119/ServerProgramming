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

struct RemoteClient {
	shared_ptr<thread> thread;  // 클라이언트가 처리되는 스레드
	Socket tcpConnection;		// Accept 후 생성되는 소켓
};

unordered_map<RemoteClient*, shared_ptr<RemoteClient>> remoteClients; // TCP 연결 객체들
deque<shared_ptr<RemoteClient>> remoteClientTrashcan; // 클라이언트 휴지통, 곧 삭제된다.
Semaphore mainThreadWorkCount; // 메인스레드가 깨어나 해야할 일이 있는지 판별한다.

recursive_mutex remoteClientMutex; // 전역 변수들을 보호하는 뮤텍스
recursive_mutex consoleMutex;	   // 스레드들의 콘솔 출력을 일렬로 세우기 위한 뮤텍스

// 컴파일러의 최적화를 벗어나는 volatile 변수로써 프로그램의 종료를 결정한다.
volatile bool loop = true;

shared_ptr<Socket> listenSocket; // 리스닝 소켓

void RemoteClientThread(shared_ptr<RemoteClient> remoteClient);
void ListenSocketThread();

void signal_handler(int signal)
{
	if (signal == SIGINT) {
		loop = false;
		mainThreadWorkCount.Notify();
	}
}

void BlockingSocketMultithread()
{
	// 컴퓨터가 멈추지 않기 위해 프로세스의 우선순위를 의도적으로 낮춘다.
	SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);

	try {
		// ctrl + c 를 누르면 루프를 종료한다.
		std::signal(SIGINT, signal_handler);

		listenSocket = make_shared<Socket>(SocketType::TCP);
		listenSocket->Bind(EndPoint("0.0.0.0", 5555));
		listenSocket->Listen();

		cout << "서버가 시작되었습니다.\n";
		cout << "CTRL + C 를 누르면 프로그램을 종료합니다.\n";

		// 리스닝 소켓을 위한 스레드
		thread tcpListenThread(ListenSocketThread);

		while (loop)
		{
			// 할 일이 생길 때까지 메인 스레드는 기다린다.
			mainThreadWorkCount.Wait();

			lock_guard<recursive_mutex> lock(remoteClientMutex);

			while (!remoteClientTrashcan.empty())
			{
				auto remoteClientToDelete = remoteClientTrashcan.front();
				// 지워져야 할 객체의 TCP 소켓을 확실하게 종료하고
				// 스레드가 종료될 때까지 기다린다.
				remoteClientToDelete->tcpConnection.Close();
				remoteClientToDelete->thread->join();

				// 클라이언트 목록에서 해당 객체를 삭제한다.
				remoteClients.erase(remoteClientToDelete.get());
				remoteClientTrashcan.pop_front();

				lock_guard<recursive_mutex> lock(consoleMutex);
				cout << "Client left. There are " << remoteClients.size() << " connections.\n";
			}
		}

		// 루프가 끝났기 때문에 프로그램을 종료한다.
		listenSocket->Close();
		{
			// 혹여나 남아있는 클라이언트도 전부 닫아버린다.
			lock_guard<recursive_mutex> lock(remoteClientMutex);
			for (auto i : remoteClients)
			{
				i.second->tcpConnection.Close();
				i.second->thread->join();
			}
		}

		// 리스닝 스레드를 종료시킨다.
		tcpListenThread.join();
		// 클라이언트 목록을 비운다.
		remoteClients.clear();
	}
	catch (Exception& ex)
	{
		cout << "Exception! " << ex.ErrorLog() << '\n';
	}
}

// 이미 연결된 TCP 소켓을 처리하는 스레드
void RemoteClientThread(shared_ptr<RemoteClient> remoteClient)
{
	while (loop)
	{
		// 에코를 받아서 그대로 넘겨준다.
		int ec = remoteClient->tcpConnection.Receive();
		if (ec <= 0) break;

		// 스트림의 모든 데이터가 전달되었는지 확인하는 작업은 생략
		remoteClient->tcpConnection.Send(remoteClient->tcpConnection.mReceiveBuffer, ec);
	}

	// 루프가 종료되면 현재 객체를 휴지통에 넣는다.
	remoteClient->tcpConnection.Close();
	lock_guard<recursive_mutex> lock(remoteClientMutex);
	remoteClientTrashcan.push_back(remoteClient);
	mainThreadWorkCount.Notify();
}

// 연결 요청이 들어올 때마다 TCP연결 소켓을 생성하는 스레드
void ListenSocketThread()
{
	while (loop)
	{
		auto remoteClient = make_shared<RemoteClient>();

		// TCP 연결이 올 때까지 기다린다.
		string errorText;
		if (listenSocket->Accept(remoteClient->tcpConnection, errorText) == 0)
		{
			// 연결이 되면 RemoteClient 객체를 만든다.
			{
				lock_guard<recursive_mutex> lock(remoteClientMutex);
				// TCP 클라이언트를 객체 원본을 key로, 포인터를 value로 넣는다.  
				remoteClients.insert({ remoteClient.get(), remoteClient });

				// TCP 연결 하나당 하나의 스레드를 만든다.
				remoteClient->thread = make_shared<thread>(RemoteClientThread, remoteClient);
			}

			// 콘솔 출력
			{
				lock_guard<recursive_mutex> lock(consoleMutex);
				cout << "Client joined. There are " << remoteClients.size() << " connections.\n";
			}
		}
		else {
			// Accept 실패
			break;
		}
	}
}

