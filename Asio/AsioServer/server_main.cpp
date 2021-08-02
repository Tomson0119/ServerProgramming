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
		return true;
	}

	virtual void OnClientDisconnect(std::shared_ptr<net::connection<CustomMsgTypes>> client)
	{

	}

	virtual void OnMessage(std::shared_ptr<net::connection<CustomMsgTypes>> client, net::message<CustomMsgTypes>& msg)
	{

	}
};

int main()
{
	CustomServer server(ServerPort);
	server.Start();

	while (1)
	{
		server.Update();
	}
}