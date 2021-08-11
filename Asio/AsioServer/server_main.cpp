#include <iostream>
#include "netCommon.h"

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

class CustomServer : public net::server_side<CustomMsgTypes>
{
public:
	CustomServer(uint16_t port)
		: net::server_side<CustomMsgTypes>(port)
	{

	}

	virtual ~CustomServer()
	{

	}

protected:
	virtual bool OnClientConnect(std::shared_ptr<net::connection<CustomMsgTypes>> client)
	{
		net::message<CustomMsgTypes> msg;
		msg.header.id = CustomMsgTypes::ServerAccept;
		client->Send(msg);

		return true;
	}

	virtual void OnClientDisconnect(std::shared_ptr<net::connection<CustomMsgTypes>> client)
	{
		cout << "Removing client [" << client->GetID() << "]\n";
	}

	virtual void OnMessage(std::shared_ptr<net::connection<CustomMsgTypes>> client, net::message<CustomMsgTypes>& msg)
	{
		switch (msg.header.id)
		{
		case CustomMsgTypes::ServerPing:
		{
			cout << "[" << client->GetID() << "]: Server Ping\n";

			// Simply bounce message back to client
			client->Send(msg);
		}
		break;

		case CustomMsgTypes::MessageAll:
		{
			string s;
			msg >> s;

			std::cout << "[" << client->GetID() << "] Message All -> " << s << '\n';			

			net::message<CustomMsgTypes> newMsg;
			newMsg.header.id = CustomMsgTypes::ServerMessage;
			newMsg << client->GetID() << s;

			MessageAllClients(newMsg, client);
		}
		break;
		}
	}
};

int main()
{
	CustomServer server(ServerPort);
	server.Start();

	bool current_key[3] = { false, false, false };
	bool previous_key[3] = { false, false, false };

	while (1)
	{
		// Process message as many as possible.
		// Server will wait if there's no messages
		server.Update(-1, true);	
	}
}