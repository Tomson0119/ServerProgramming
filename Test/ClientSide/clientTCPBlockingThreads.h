#pragma once

#include "../Network/common.h"
#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>


using namespace std;
using namespace std::chrono_literals;

const int MaxClientNum = 10;

void BlockingSocketMultithread()
{
	// ��ǻ�Ͱ� ������ �ʵ��� ���μ����� �켱������ �ǵ������� �����.
	SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);

	vector<shared_ptr<thread>> threads;
	int64_t totalReceivedBytes = 0;
	int connectedClientCount = 0;
	recursive_mutex mutex;

	for (int i = 0; i < MaxClientNum; ++i)
	{
		shared_ptr<thread> t = make_shared<thread>(
			[&totalReceivedBytes, &connectedClientCount, &mutex]()
			{
				try {
					Socket sck(SocketType::TCP);
					sck.Bind(EndPoint::Any);
					sck.Connect(EndPoint("127.0.0.1", 5555));
					{
						// ������ �Ǹ� ����� Ŭ���̾�Ʈ�� ���� ������Ų��.
						lock_guard<recursive_mutex> lock(mutex);
						connectedClientCount++;
					}

					string receivedData;
					while (true)
					{
						string data = "Hello World.";
						sck.Send(data);
						int len = sck.Receive();

						// ���ῡ ������ �߻��ϸ� �۾��� ������.
						if (len <= 0) break;

						// ���������� ���� �������� ����Ʈ ���� �����Ѵ�.
						lock_guard<recursive_mutex> lock(mutex);
						totalReceivedBytes += len;
					}
				}
				catch (Exception& ex)
				{
					// ������ �о���� ������ �Ӱ� ����Ѵ�.
					lock_guard<recursive_mutex> lock(mutex);
					cout << ex.ErrorLog() << '\n';
				}

				lock_guard<recursive_mutex> lock(mutex);
				connectedClientCount--;
			});

		lock_guard<recursive_mutex> lock(mutex);
		threads.push_back(t);
	}

	while (true)
	{
		{
			lock_guard<recursive_mutex> lock(mutex);
			cout << "Total echoed bytes : " << (uint64_t)totalReceivedBytes
				<< ", thread count : " << connectedClientCount << '\n';
		}
		this_thread::sleep_for(2s);
	}
}