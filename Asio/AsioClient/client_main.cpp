#include <netCommon.h>
#include <iostream>

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
	void PingServer()
	{
		net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerPing;

		// Don't use this in business
		chrono::system_clock::time_point timeNow = chrono::system_clock::now();

		msg << timeNow;
		Send(msg);
	}

	void MessageAll()
	{
		net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::MessageAll;
		Send(msg);
	}
};

int main()
{
	CustomClient client;
	client.Connect("127.0.0.1", 5505);

	bool key[3] = { false, false, false };
	bool old_key[3] = { false, false, false };
	
	bool bQuit = false;
	while (!bQuit)
	{
		// If console is focused
		if (GetForegroundWindow() == GetConsoleWindow())
		{
			key[0] = GetAsyncKeyState('1') & 0x8000;
			key[1] = GetAsyncKeyState('2') & 0x8000;
			key[2] = GetAsyncKeyState('3') & 0x8000;
		}

		if (key[0] && !old_key[0]) client.PingServer();
		if (key[1] && !old_key[1]) client.MessageAll();
		if (key[2] && !old_key[2]) bQuit = true;

		for (int i = 0; i < 3; ++i) 
			old_key[i] = key[i];

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
					msg >> clientID;
					std::cout << "Hello from [" << clientID << "]\n";
				}
				break;
				}
			}
		}
		else
		{
			std::cout << "Server Down\n";
			bQuit = true;
		}
	}
}