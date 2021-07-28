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

// �����Ϸ��� ����ȭ�� ����� volatile �����ν� ���α׷��� ���Ḧ �����Ѵ�.
volatile bool loop = true;

void signal_handler(int signal)
{
	if (signal == SIGINT) {
		loop = false;
	}
}

void NonBlockingSocket()
{
	// ctrl + c �� ������ ������ �����Ѵ�.
	std::signal(SIGINT, signal_handler);

	try {
		struct RemoteClient
		{
			Socket tcpConnection;	// Accept�� TCP ��ü
		};
		unordered_map<RemoteClient*, shared_ptr<RemoteClient>> remoteClients;

		Socket listen(SocketType::TCP);
		listen.Bind(EndPoint("0.0.0.0", 5555));
		listen.SetNonBlocking(); // ���� �������� �����Ѵ�.
		listen.Listen();

		cout << "������ ���۵Ǿ����ϴ�.\n";
		cout << "CTRL + C �� ������ ���α׷��� �����մϴ�.\n";

		// ������ ���ϰ� ���� ���� ��� I/O ���� �̺�ũ�� �߻��� ������ ��ٸ���.
		
		// ���� �ڵ鿡 ���ؼ� poll�� �Ѵ�.
		vector<PollFD> readFds;
		// ��� ������ ��� RemoteClient�� ���� ������ ����Ų��.
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

			// 100ms ���� readFds�� pollFd �� �̺�Ʈ�� ������ ������ ��ٸ���.
			// ���� �� readFds�� �̺�Ʈ�� �߻��� pollfd�� revent�� 0�� �ƴ� ���̴�.
			Poll(readFds.data(), (int)readFds.size(), 100);
			
			// readFds�� �����ؼ� �ʿ��� ���� ó���Ѵ�.
			int num = 0;
			for (auto fd : readFds)
			{
				if (fd.mPollfd.revents != 0)
				{
					if (num == readFds.size() - 1) // ������ ����
					{
						auto remoteClient = make_shared<RemoteClient>();

						// Ŭ���̾�Ʈ ������ ���� �����̹Ƿ� �׳� ȣ�⸸ �ص� ��
						string ignore;
						listen.Accept(remoteClient->tcpConnection, ignore);
						remoteClient->tcpConnection.SetNonBlocking();

						// �̸� Ŭ���̾�Ʈ ��Ͽ� �߰��Ѵ�.
						remoteClients.insert({ remoteClient.get(), remoteClient });
						cout << "Client joined. There are " << remoteClients.size() << " connections.\n";
					}
					else {  // TCP ���� ����
						// �ش� �ε����� Ŭ���̾�Ʈ ��ü�� ���´�.
						RemoteClient* remoteClient = readFdsToRemoteClients[num];

						int ec = remoteClient->tcpConnection.Receive();
						if (ec <= 0)
						{
							// ������ �߻������Ƿ� �ش� ������ ����
							remoteClient->tcpConnection.Close();
							remoteClients.erase(remoteClient);

							cout << "Client left. There are " << remoteClients.size() << " connections.\n";
						}
						else {
							// ���� �����͸� �״�� �۽��Ѵ�.
							// ������ ����ġ�� ������ ����
							remoteClient->tcpConnection.Send(remoteClient->tcpConnection.mReceiveBuffer, ec);
						}
					}
				}
				num++;
			}			
		}

		// ������ ���������� ��� ����.
		listen.Close();
		remoteClients.clear();
	}
	catch (Exception& ex)
	{
		cout << "Exception! " << ex.ErrorLog() << '\n';
	}
}
