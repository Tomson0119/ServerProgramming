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
	// 컴퓨터가 멈추지 않도록 프로세스의 우선순위를 의도적으로 낮춘다.
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
						// 연결이 되면 연결된 클라이언트의 수를 증가시킨다.
						lock_guard<recursive_mutex> lock(mutex);
						connectedClientCount++;
					}

					string receivedData;
					while (true)
					{
						string data = "Hello World.";
						sck.Send(data);
						int len = sck.Receive();

						// 연결에 문제가 발생하면 작업을 끝낸다.
						if (len <= 0) break;

						// 서버측에서 받은 데이터의 바이트 수를 누적한다.
						lock_guard<recursive_mutex> lock(mutex);
						totalReceivedBytes += len;
					}
				}
				catch (Exception& ex)
				{
					// 에러를 읽어들일 때에도 임계 잠금한다.
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