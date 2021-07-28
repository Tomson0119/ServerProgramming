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

// ������ ������ �� �ִ� �÷��� ����
// �������� ����� �� �����Ƿ� volatile�� �����Ѵ�.
volatile bool loop = true;

void ProcessSignalAction(int number)
{
	if (number == SIGINT)
		loop = false;
}

// TCP ���� ��ü
class RemoteClient
{
public:
	shared_ptr<thread> thrd; // Ŭ���̾�Ʈ�� �����ϴ� ������
	Socket tcp;				 // Accept�� TPC ���� ��ü
	RemoteClient() { }
	RemoteClient(SocketType type, bool overlapped) : tcp(type, overlapped) { }
};

unordered_map<RemoteClient*, shared_ptr<RemoteClient>> remoteClients;

void ProcessClientLeave(shared_ptr<RemoteClient> client)
{
	// ���� Ȥ�� ����� �����̴�.
	// ���� �� ������ �����Ѵ�.
	client->tcp.Close();
	remoteClients.erase(client.get());

	cout << "Client Left. There are " << remoteClients.size() << " connections.\n";
}

void IOCPSocketExample()
{
	// CTRL + C �� ������ ����ȴ�.
	signal(SIGINT, ProcessSignalAction);

	try {
		// �����带 �ϳ��� ����.
		IOCP iocp(1);

		Socket listening(SocketType::TCP, true);
		listening.Bind(EndPoint("0.0.0.0", 5555));
		listening.Listen();

		// ������ ������ iocp�� �ִ´�.
		iocp.Add(listening, nullptr);

		// Ŭ���̾�Ʈ �ĺ� ��ü�� �����.
		auto remoteClientCandidate = make_shared<RemoteClient>(SocketType::TCP, true);

		// Ŭ���̾�Ʈ �ĺ��� �Ѱ��ְ�, accept�� �Ǿ��� ��
		// Pending ����(���� ó�� ���� ����)������ Ȯ���Ѵ�.
		string errorText;
		if (!listening.AcceptOverlapped(remoteClientCandidate->tcp, errorText)
			&& WSAGetLastError() != ERROR_IO_PENDING)
		{
			throw Exception("Overlapped AcceptEx Failed");
		}
		listening.mIsReadOverlapped = true;

		cout << "������ ���۵Ǿ����ϴ�.\n";
		cout << "CTRL + C�� ������ ���α׷��� �����մϴ�.\n";

		while (loop)
		{
			// I/O �Ϸ� �̺�Ʈ�� ���� ������ ��ٸ���.
			IocpEvents readEvents;
			iocp.Wait(readEvents, 100);

			// ���� �̺�Ʈ�� ó���Ѵ�.
			for (int i = 0; i < readEvents.mEventCount; ++i)
			{
				auto& readEvent = readEvents.mEvents[i];
				if (readEvent.lpCompletionKey == 0) // ������ ������ �ǹ�
				{
					listening.mIsReadOverlapped = false;
					if (remoteClientCandidate->tcp.UpdateAcceptContext(listening)!=0)
					{
						// ������Ʈ�� �����ϸ� ������ ������ �ݴ´�.
						listening.Close();
					}
					else
					{
						auto remoteClient = remoteClientCandidate;

						// ���ο� TCP ������ IOCP�� �߰��Ѵ�.
						iocp.Add(remoteClient->tcp, remoteClient.get());

						if (remoteClient->tcp.ReceiveOverlapped() != 0
							&& WSAGetLastError() != ERROR_IO_PENDING)
						{
							// ���� ����
							remoteClient->tcp.Close();
						}
						else
						{
							// �ϷḦ ����ϴ� �� ���·� �ٲ۴�.
							remoteClient->tcp.mIsReadOverlapped = true;
							
							// Ŭ���̾�Ʈ ��Ͽ� �߰�
							remoteClients.insert({ remoteClient.get(), remoteClient });
							cout << "Client joined. There are " << remoteClients.size() << " connections.\n";
						}

						// ������ �����ߴٸ� �ٽ� ������ ���� ���°� �Ǿ���ϹǷ�
						// �ٽ� overlapped I/O�� �Ǵ�.
						remoteClientCandidate = make_shared<RemoteClient>(SocketType::TCP, true);
						string errorText;
						if (!listening.AcceptOverlapped(remoteClientCandidate->tcp, errorText)
							&& WSAGetLastError() != ERROR_IO_PENDING)
						{
							// ������ �߻��ϸ� �������� �����Ѵ�.
							listening.Close();
						}
						else
						{
							// ���ο� ������ ��ٸ��� ���°� �ȴ�.
							listening.mIsReadOverlapped = true;
						}
					}
				}
				else // TCP ���� ����
				{
					// ���� �����͸� �״�� ȸ���Ѵ�.
					shared_ptr<RemoteClient> remoteClient = remoteClients[(RemoteClient*)readEvent.lpCompletionKey];
					
					// ���Ź��� �������� ũ�Ⱑ 0 ���̶�� ������ ���Ѱ�
					if (readEvent.dwNumberOfBytesTransferred <= 0)
					{
						int a = 0;
					}
					
					if (remoteClient)
					{
						// ���� �Ϸ�� �����͸� ����Ѵ�.
						remoteClient->tcp.mIsReadOverlapped = false;
						int ec = readEvent.dwNumberOfBytesTransferred;

						if (ec <= 0)
						{
							// ���̰� 0�̹Ƿ� ���� ����.
							// Ȥ�� ������ ����
							ProcessClientLeave(remoteClient);
						}
						else {
							char* echoData = remoteClient->tcp.mReceiveBuffer;

							// ��Ģ������ ��Ʈ���� ������ �Ǻ��� ���� �����ؾ��Ѵ�.
							// �׸��� Send �Լ��� Overlapped Send�� �ϴ� ���� �ٶ����ϴ�.
							remoteClient->tcp.Send(echoData, ec);

							// �ٽ� ���Źޱ� ���ؼ� overlapped I/O�� �Ǵ�.
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

		// CTRL + C�� ������ ������ �����ߴ�.
		// ������ I/O�� ��� �Ϸ�Ǳ� ������ �׳� ������ �ȵȴ�.
		listening.Close();
		for (auto p : remoteClients)
			p.second->tcp.Close();

		cout << "������ �����ϰ� �ֽ��ϴ�...\n";
		while (remoteClients.size() > 0 || listening.mIsReadOverlapped)
		{
			// I/O completion�� ���� ������ Ŭ���̾�Ʈ�� �����Ѵ�.
			for (auto i = remoteClients.begin(); i != remoteClients.end();)
			{
				if (!i->second->tcp.mIsReadOverlapped)
					i = remoteClients.erase(i);
				else
					i++;
			}

			// I/O completion�� �߻��ϸ� �� �̻� I/O�� ���� �ʰ� ���� �÷��׸� �ִ´�.
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

		cout << "���� ����.\n";
	}
	catch (Exception& ex)
	{
		cout << ex.ErrorLog() << '\n';
	}
}
