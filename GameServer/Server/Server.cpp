#include "common.h"
#include "Server.h"
#include "Session.h"

std::unordered_map<int, std::unique_ptr<Session>> Server::gClients;

void Socket::SendRoutine(DWORD err, DWORD bytes, LPWSAOVERLAPPED overlapped, DWORD flag)
{
	WSAOVERLAPPEDEX* sendOverEx = reinterpret_cast<WSAOVERLAPPEDEX*>(overlapped);
	Session* client = reinterpret_cast<Session*>(sendOverEx->Caller);
	std::cout << "Sent " << bytes << " bytes to [" << client->ID << "] client.\n";
	delete sendOverEx;
}

void Socket::RecvRoutine(DWORD err, DWORD bytes, LPWSAOVERLAPPED overlapped, DWORD flag)
{
	WSAOVERLAPPEDEX* recvOverEx = reinterpret_cast<WSAOVERLAPPEDEX*>(overlapped);
	Session* client = reinterpret_cast<Session*>(recvOverEx->Caller);

	if (bytes == 0) {
		std::cout << "[" << client->ID << "] client has leaved.\n";
		Server::gClients.erase(client->ID);
		return;
	}

	std::cout << "[" << client->ID << "] client sent " << bytes << " bytes.\n";
	Server::HandleMessage(client->mReceiveBuffer, client->ID);
	client->RecvMsg();
}

Server::Server(Protocol type, const EndPoint& ep)
	: mEndPoint(ep), mLoop(true)
{
	mListenSck = std::make_unique<Socket>(type);
}

Server::~Server()
{
}

void Server::Run()
{
	mListenSck->Bind(mEndPoint);
	mListenSck->Listen();
	std::cout << "Listening to clients...\n";

	int i = 0;
	while (mLoop)
	{
		if (gClients.size() < 10) {
			SOCKET client = mListenSck->Accept(mEndPoint);
			gClients.emplace(i, std::make_unique<Session>(i, client));
			SendClientsPosition(i);

			std::cout << "[" << i << "] client has joined...\n";

			gClients[i]->RecvMsg();

			i += 1;
		}
		else
			SleepEx(100, true);
	}
}

void Server::HandleMessage(Message& msg, int id)
{
	switch (msg.Header.msg_type)
	{
	case MsgType::MSG_MOVE:
		uint8_t command = msg.Body[0];
		uint8_t x = msg.Body[1];
		uint8_t y = msg.Body[2];
		
		if (!gClients[id]->IsSame({ x,y }))
		{
			std::cout << "[" << id << "] client banned: inappropriate position.\n";
			gClients.erase(id);
			return;
		}

		switch (command)
		{
		case VK_LEFT:
			if (x > 0 && CheckIfValidPosition({ (uint8_t)(x - 1), y }))
				x -= 1;
			break;

		case VK_RIGHT:
			if (x < MaxBoardSize-1 && CheckIfValidPosition({ (uint8_t)(x + 1), y }))
				x += 1;
			break;

		case VK_UP:
			if (y < MaxBoardSize - 1 && CheckIfValidPosition({ x, (uint8_t)(y + 1) }))
				y += 1;
			break;

		case VK_DOWN:
			if (y > 0 && CheckIfValidPosition({ x, (uint8_t)(y-1) }))
				y -= 1;
			break;
		}
		gClients[id]->PlayerPos = { x,y };

		Message move_msg(MsgType::MSG_MOVE);
		move_msg.Push((uint8_t)id);
		move_msg.Push(x);
		move_msg.Push(y);

		for (auto& c : gClients)
			c.second->SendMsg(move_msg);
	}
}

bool Server::CheckIfValidPosition(PlayerCoord coord)
{
	for (auto& c : gClients)
	{
		if (c.second->IsSame(coord))
			return false;
	}
	return true;
}

PlayerCoord Server::PeekNewPosition(int id)
{
	PlayerCoord p{ 0,0 };

	int i = 0;
	while (i < MaxBoardSize*MaxBoardSize) {
		p.Col = i % MaxBoardSize;
		p.Row = i / MaxBoardSize;

		bool found = true;
		for (auto& c : gClients)
		{
			if (c.first != id) {
				PlayerCoord cc = c.second->PlayerPos;
				if (cc.Col == p.Col && cc.Row == p.Row) {
					found = false;
					break;
				}
			}
		}

		if (found) break;
		i++;
	}
	return p;
}

void Server::SendClientsPosition(int id)
{
	gClients[id]->PlayerPos = PeekNewPosition(id);
	
	Message join_msg(MsgType::MSG_JOIN);
	join_msg.Push(gClients[id]->PlayerPos.Col);
	join_msg.Push(gClients[id]->PlayerPos.Row);

	Message accept_msg(MsgType::MSG_ACCEPT);
	accept_msg.Push((uint8_t)id);
	for (auto& c : gClients)
	{
		if (c.first != id)
			c.second->SendMsg(join_msg);

		PlayerCoord p = c.second->PlayerPos;
		accept_msg.Push(p.Col);
		accept_msg.Push(p.Row);
	}
	gClients[id]->SendMsg(accept_msg);

	
}
