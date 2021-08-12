#include <netCommon.h>
#include <iostream>
#include <conio.h>
#include <fstream>

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

	string userName;

public:
	CustomClient()
	{
	}

	virtual ~CustomClient()
	{
		if (inputThr.joinable()) inputThr.join();
		if (outputThr.joinable()) outputThr.join();
		if (printThr.joinable()) printThr.join();
	}

	void SetUserName()
	{
		cout << "Insert your name: ";
		getline(cin, userName);
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
		scoped_lock<mutex,mutex> lock(mtxDirty, mtxPrint);
		dirty_flag = true;

		printQue.push_back(s);
		
		if (printQue.size() > 10)
			printQue.pop_front();
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

		msg.encodeString(s, s.size());
		msg << (uint16_t)s.size();
		msg.encodeString(userName, userName.size());
		msg << (uint16_t)userName.size();

		Send(msg);
	}
};

int main()
{
	CustomClient client;
	client.Connect("14.38.228.31", 5505);
	
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
					client.SetUserName();
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
					uint16_t userNameLen;
					msg >> userNameLen;

					string userName;
					userName.resize(userNameLen);
					msg.decodeString(userName, userNameLen);

					uint16_t msgLen;
					msg >> msgLen;

					string message;
					message.resize(msgLen);
					msg.decodeString(message, msgLen);

					client.AppendMsg("[" + userName + "]  :  " + message);
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