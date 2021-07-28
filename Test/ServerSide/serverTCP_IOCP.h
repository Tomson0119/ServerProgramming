#pragma once

#include "../Network/common.h"
#include <iostream>
#include <csignal>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <memory>
#include <vector>

using namespace std;

// 루프를 종료할 수 있는 플래그 변수
// 언제든지 변경될 수 있으므로 volatile로 선언한다.
volatile bool loop = true;

void ProcessSignalAction(int number)
{
	if (number == SIGINT)
		loop = false;
}

// TCP 연결 객체
class RemoteClient
{
public:
	shared_ptr<thread> thrd; // 클라이언트를 수행하는 스레드
	Socket tcp;				 // Accept한 TPC 연결 객체
	RemoteClient() { }
	RemoteClient(SocketType type, bool overlapped) : tcp(type, overlapped) { }
};

unordered_map<RemoteClient*, shared_ptr<RemoteClient>> remoteClients;

void ProcessClientLeave(shared_ptr<RemoteClient> client)
{
	// 에러 혹은 종료된 소켓이다.
	// 따라서 이 소켓은 제거한다.
	client->tcp.Close();
	remoteClients.erase(client.get());

	cout << "Client Left. There are " << remoteClients.size() << " connections.\n";
}

void IOCPSocketExample()
{
	// CTRL + C 를 누르면 종료된다.
	signal(SIGINT, ProcessSignalAction);

	try {
		// 스레드를 하나만 쓴다.
		IOCP iocp(1);

		Socket listening(SocketType::TCP, true);
		listening.Bind(EndPoint("0.0.0.0", 5555));
		listening.Listen();

		// 리스닝 소켓을 iocp에 넣는다.
		iocp.Add(listening, nullptr);

		// 클라이언트 후보 객체를 만든다.
		auto remoteClientCandidate = make_shared<RemoteClient>(SocketType::TCP, true);

		// 클라이언트 후보를 넘겨주고, accept가 되었는 지
		// Pending 상태(아직 처리 중인 상태)인지를 확인한다.
		string errorText;
		if (!listening.AcceptOverlapped(remoteClientCandidate->tcp, errorText)
			&& WSAGetLastError() != ERROR_IO_PENDING)
		{
			throw Exception("Overlapped AcceptEx Failed");
		}
		listening.mIsReadOverlapped = true;

		cout << "서버가 시작되었습니다.\n";
		cout << "CTRL + C를 누르면 프로그램을 종료합니다.\n";

		while (loop)
		{
			// I/O 완료 이벤트가 있을 때까지 기다린다.
			IocpEvents readEvents;
			iocp.Wait(readEvents, 100);

			// 받은 이벤트를 처리한다.
			for (int i = 0; i < readEvents.mEventCount; ++i)
			{
				auto& readEvent = readEvents.mEvents[i];
				if (readEvent.lpCompletionKey == 0) // 리스닝 소켓을 의미
				{
					listening.mIsReadOverlapped = false;
					if (remoteClientCandidate->tcp.UpdateAcceptContext(listening)!=0)
					{
						// 업데이트가 실패하면 리스닝 소켓을 닫는다.
						listening.Close();
					}
					else
					{
						auto remoteClient = remoteClientCandidate;

						// 새로운 TCP 소켓을 IOCP에 추가한다.
						iocp.Add(remoteClient->tcp, remoteClient.get());

						if (remoteClient->tcp.ReceiveOverlapped() != 0
							&& WSAGetLastError() != ERROR_IO_PENDING)
						{
							// 수신 에러
							remoteClient->tcp.Close();
						}
						else
						{
							// 완료를 대기하는 중 상태로 바꾼다.
							remoteClient->tcp.mIsReadOverlapped = true;
							
							// 클라이언트 목록에 추가
							remoteClients.insert({ remoteClient.get(), remoteClient });
							cout << "Client joined. There are " << remoteClients.size() << " connections.\n";
						}

						// 소켓을 생성했다면 다시 소켓을 받을 상태가 되어야하므로
						// 다시 overlapped I/O를 건다.
						remoteClientCandidate = make_shared<RemoteClient>(SocketType::TCP, true);
						string errorText;
						if (!listening.AcceptOverlapped(remoteClientCandidate->tcp, errorText)
							&& WSAGetLastError() != ERROR_IO_PENDING)
						{
							// 에러가 발생하면 리스닝을 종료한다.
							listening.Close();
						}
						else
						{
							// 새로운 연결을 기다리는 상태가 된다.
							listening.mIsReadOverlapped = true;
						}
					}
				}
				else // TCP 연결 소켓
				{
					// 받은 데이터를 그대로 회신한다.
					shared_ptr<RemoteClient> remoteClient = remoteClients[(RemoteClient*)readEvent.lpCompletionKey];
					
					// 수신받은 데이터의 크기가 0 밑이라면 수신을 못한것
					if (readEvent.dwNumberOfBytesTransferred <= 0)
					{
						int a = 0;
					}
					
					if (remoteClient)
					{
						// 수신 완료된 데이터를 사용한다.
						remoteClient->tcp.mIsReadOverlapped = false;
						int ec = readEvent.dwNumberOfBytesTransferred;

						if (ec <= 0)
						{
							// 길이가 0이므로 연결 종료.
							// 혹은 음수라서 에러
							ProcessClientLeave(remoteClient);
						}
						else {
							char* echoData = remoteClient->tcp.mReceiveBuffer;

							// 원칙적으로 스트림은 데이터 판별을 먼저 진행해야한다.
							// 그리고 Send 함수도 Overlapped Send를 하는 것이 바람직하다.
							remoteClient->tcp.Send(echoData, ec);

							// 다시 수신받기 위해서 overlapped I/O를 건다.
							if (remoteClient->tcp.ReceiveOverlapped() != 0
								&& WSAGetLastError() != ERROR_IO_PENDING)
							{
								ProcessClientLeave(remoteClient);
							}
							else
							{
								remoteClient->tcp.mIsReadOverlapped = true;
							}
						}
					}
				}
			}
		}

		// CTRL + C를 눌러서 루프를 종료했다.
		// 하지만 I/O가 모두 완료되기 전까지 그냥 나가면 안된다.
		listening.Close();
		for (auto p : remoteClients)
			p.second->tcp.Close();

		cout << "서버를 종료하고 있습니다...\n";
		while (remoteClients.size() > 0 || listening.mIsReadOverlapped)
		{
			// I/O completion이 없는 상태의 클라이언트를 제거한다.
			for (auto i = remoteClients.begin(); i != remoteClients.end();)
			{
				if (!i->second->tcp.mIsReadOverlapped)
					i = remoteClients.erase(i);
				else
					i++;
			}

			// I/O completion이 발생하면 더 이상 I/O를 걸지 않고 정리 플래그를 넣는다.
			IocpEvents readEvents;
			iocp.Wait(readEvents, 100);

			for (int i = 0; i < readEvents.mEventCount; ++i)
			{
				auto& readEvent = readEvents.mEvents[i];
				if (readEvent.lpCompletionKey == 0)
					listening.mIsReadOverlapped = false;
				else
				{
					auto remoteClient = remoteClients[(RemoteClient*)readEvent.lpCompletionKey];
					if (remoteClient)
						remoteClient->tcp.mIsReadOverlapped = false;
				}
			}
		}

		cout << "서버 종료.\n";
	}
	catch (Exception& ex)
	{
		cout << ex.ErrorLog() << '\n';
	}
}
