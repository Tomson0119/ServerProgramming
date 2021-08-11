#include <netCommon.h>
#include <iostream>
#include <conio.h>

using namespace std;

constexpr int ServerPort = 5505;

enum class CustomMsgTypes : uint32_t
{
	ServerAccept,
	ServerDeny,
	ServerPing,
	MessageAll,
	ServerMessage
};

class CustomClient : public net::client_side<CustomMsgTypes>
{
public:
	atomic_bool loop = true;

private:
	bool dirty_flag = true;
	mutex mtxDirty;

	string inputData = "";
	mutex mtxString;

	deque<string> printQue;
	mutex mtxPrint;

	thread inputThr, outputThr, printThr;
	
public:
	~CustomClient()
	{
		if (inputThr.joinable()) inputThr.join();
		if (outputThr.joinable()) outputThr.join();
		if (printThr.joinable()) printThr.join();
	}

	void AssignThreads()
	{
		inputThr = thread([&]()
			{
				while (IsLoop())
				{
					if (GetForegroundWindow() == GetConsoleWindow()) {
						int c = _getch();

						scoped_lock<mutex, mutex> lock(mtxDirty, mtxString);						
						if ((int)c == 13) // Enter
						{
							if (inputData == "exit")
							{
								loop.store(false, memory_order_release);
							}
							MessageAll(inputData);
							inputData.clear();
						}
						else if ((int)c == 8 && !inputData.empty()) // backspace
						{
							inputData.erase(inputData.end() - 1);
						}
						else if ((int)c != 27) // Prevent ESC input.
						{
							inputData += c;
						}

						dirty_flag = true;
					}
				}
			});

		printThr = thread([&]()
			{
				while (IsLoop())
				{
					scoped_lock<mutex, mutex, mutex> lock(mtxDirty, mtxString, mtxPrint);
					if (dirty_flag) {
						system("cls");
						cout << "Input: " << inputData << '\n';

						for (auto iter = printQue.begin(); iter != printQue.end(); ++iter)
						{
							cout << *iter << endl;
						}
						dirty_flag = false;
					}
				}
			});
	}

	bool IsLoop()
	{
		return loop.load(memory_order_acquire);
	}

	void AppendMsg(const string& s)
	{
		lock_guard<mutex> lock(mtxPrint);
		printQue.push_back(s);
	}

	void PingServer()
	{
		net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerPing;

		// Don't use this in business
		chrono::system_clock::time_point timeNow = chrono::system_clock::now();

		msg << timeNow;
		Send(msg);
	}

	void MessageAll(const string& s)
	{
		net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::MessageAll;

		if(s.size() > 0)
			msg << s;
		Send(msg);
	}
};

int main()
{
	CustomClient client;
	client.Connect("127.0.0.1", 5505);
	
	while (true)
	{
		// Client exit..
		if (!client.IsLoop())
			break;

		if (client.IsConnected())
		{
			if (!client.Incoming().empty())
			{
				auto msg = client.Incoming().pop_front().msg;

				switch (msg.header.id)
				{
				case CustomMsgTypes::ServerAccept:
				{
					std::cout << "Server Accepted Connection.\n";
					client.AssignThreads();
				}
				break;

				case CustomMsgTypes::ServerPing:
				{
					auto timeNow = chrono::system_clock::now();
					chrono::system_clock::time_point timeThen;
					msg >> timeThen;
					cout << "Ping: " << chrono::duration<double>(timeNow - timeThen).count() << '\n';
				}
				break;

				case CustomMsgTypes::ServerMessage:
				{
					// Server has responded to ping request
					uint32_t clientID;
					string message;
					msg >> clientID >> message;
					client.AppendMsg(message);
				}
				break;
				}
			}
		}
		else
		{
			std::cout << "Server Down\n";
			client.loop.store(false, memory_order_release);
		}
	}
}