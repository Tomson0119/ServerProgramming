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
	shared_ptr<thread> thread;  // Ŭ���̾�Ʈ�� ó���Ǵ� ������
	Socket tcpConnection;		// Accept �� �����Ǵ� ����
};

unordered_map<RemoteClient*, shared_ptr<RemoteClient>> remoteClients; // TCP ���� ��ü��
deque<shared_ptr<RemoteClient>> remoteClientTrashcan; // Ŭ���̾�Ʈ ������, �� �����ȴ�.
Semaphore mainThreadWorkCount; // ���ν����尡 ��� �ؾ��� ���� �ִ��� �Ǻ��Ѵ�.

recursive_mutex remoteClientMutex; // ���� �������� ��ȣ�ϴ� ���ؽ�
recursive_mutex consoleMutex;	   // ��������� �ܼ� ����� �Ϸķ� ����� ���� ���ؽ�

// �����Ϸ��� ����ȭ�� ����� volatile �����ν� ���α׷��� ���Ḧ �����Ѵ�.
volatile bool loop = true;

shared_ptr<Socket> listenSocket; // ������ ����

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
	// ��ǻ�Ͱ� ������ �ʱ� ���� ���μ����� �켱������ �ǵ������� �����.
	SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);

	try {
		// ctrl + c �� ������ ������ �����Ѵ�.
		std::signal(SIGINT, signal_handler);

		listenSocket = make_shared<Socket>(SocketType::TCP);
		listenSocket->Bind(EndPoint("0.0.0.0", 5555));
		listenSocket->Listen();

		cout << "������ ���۵Ǿ����ϴ�.\n";
		cout << "CTRL + C �� ������ ���α׷��� �����մϴ�.\n";

		// ������ ������ ���� ������
		thread tcpListenThread(ListenSocketThread);

		while (loop)
		{
			// �� ���� ���� ������ ���� ������� ��ٸ���.
			mainThreadWorkCount.Wait();

			lock_guard<recursive_mutex> lock(remoteClientMutex);

			while (!remoteClientTrashcan.empty())
			{
				auto remoteClientToDelete = remoteClientTrashcan.front();
				// �������� �� ��ü�� TCP ������ Ȯ���ϰ� �����ϰ�
				// �����尡 ����� ������ ��ٸ���.
				remoteClientToDelete->tcpConnection.Close();
				remoteClientToDelete->thread->join();

				// Ŭ���̾�Ʈ ��Ͽ��� �ش� ��ü�� �����Ѵ�.
				remoteClients.erase(remoteClientToDelete.get());
				remoteClientTrashcan.pop_front();

				lock_guard<recursive_mutex> lock(consoleMutex);
				cout << "Client left. There are " << remoteClients.size() << " connections.\n";
			}
		}

		// ������ ������ ������ ���α׷��� �����Ѵ�.
		listenSocket->Close();
		{
			// Ȥ���� �����ִ� Ŭ���̾�Ʈ�� ���� �ݾƹ�����.
			lock_guard<recursive_mutex> lock(remoteClientMutex);
			for (auto i : remoteClients)
			{
				i.second->tcpConnection.Close();
				i.second->thread->join();
			}
		}

		// ������ �����带 �����Ų��.
		tcpListenThread.join();
		// Ŭ���̾�Ʈ ����� ����.
		remoteClients.clear();
	}
	catch (Exception& ex)
	{
		cout << "Exception! " << ex.ErrorLog() << '\n';
	}
}

// �̹� ����� TCP ������ ó���ϴ� ������
void RemoteClientThread(shared_ptr<RemoteClient> remoteClient)
{
	while (loop)
	{
		// ���ڸ� �޾Ƽ� �״�� �Ѱ��ش�.
		int ec = remoteClient->tcpConnection.Receive();
		if (ec <= 0) break;

		// ��Ʈ���� ��� �����Ͱ� ���޵Ǿ����� Ȯ���ϴ� �۾��� ����
		remoteClient->tcpConnection.Send(remoteClient->tcpConnection.mReceiveBuffer, ec);
	}

	// ������ ����Ǹ� ���� ��ü�� �����뿡 �ִ´�.
	remoteClient->tcpConnection.Close();
	lock_guard<recursive_mutex> lock(remoteClientMutex);
	remoteClientTrashcan.push_back(remoteClient);
	mainThreadWorkCount.Notify();
}

// ���� ��û�� ���� ������ TCP���� ������ �����ϴ� ������
void ListenSocketThread()
{
	while (loop)
	{
		auto remoteClient = make_shared<RemoteClient>();

		// TCP ������ �� ������ ��ٸ���.
		string errorText;
		if (listenSocket->Accept(remoteClient->tcpConnection, errorText) == 0)
		{
			// ������ �Ǹ� RemoteClient ��ü�� �����.
			{
				lock_guard<recursive_mutex> lock(remoteClientMutex);
				// TCP Ŭ���̾�Ʈ�� ��ü ������ key��, �����͸� value�� �ִ´�.  
				remoteClients.insert({ remoteClient.get(), remoteClient });

				// TCP ���� �ϳ��� �ϳ��� �����带 �����.
				remoteClient->thread = make_shared<thread>(RemoteClientThread, remoteClient);
			}

			// �ܼ� ���
			{
				lock_guard<recursive_mutex> lock(consoleMutex);
				cout << "Client joined. There are " << remoteClients.size() << " connections.\n";
			}
		}
		else {
			// Accept ����
			break;
		}
	}
}

