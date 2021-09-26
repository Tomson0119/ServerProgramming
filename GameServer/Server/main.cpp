#include "common.h"
#include <iostream>

using namespace std;

const short SERVER_PORT = 5505;

struct PlayerCoord
{
	uint8_t Row;
	uint8_t Col;
};

PlayerCoord playerCoord{ 0,0 };

bool HandleClientMessage(Socket& client, Message& msg)
{
	switch (msg.mMsgType)
	{
	case MsgType::MSG_MOVE:
	{
		const int MaxBoardSize = 8;

		uint8_t command;
		msg.Pop(command);

		printf("[Message Move] Command: 0x%x\n", command);

		if (playerCoord.Col > 0 && command == VK_LEFT)
			playerCoord.Col -= 1;
		else if (playerCoord.Col < MaxBoardSize - 1 && command == VK_RIGHT)
			playerCoord.Col += 1;
		else if (playerCoord.Row > 0 && command == VK_DOWN)
			playerCoord.Row -= 1;
		else if (playerCoord.Row < MaxBoardSize - 1 && command == VK_UP)
			playerCoord.Row += 1;

		Message newMsg(MsgType::MSG_MOVE);
		newMsg.Push(playerCoord.Row);
		newMsg.Push(playerCoord.Col);
		client.Send(newMsg);
		break;
	}
	case MsgType::MSG_DISCONNECT:
		cout << "Client disconnected\n";
		return false;
	}
	return true;
}

int main()
{
	try {
		Socket listenSck(Protocol::TCP);
		EndPoint serverEp = EndPoint::Any(SERVER_PORT);
		listenSck.Bind(serverEp);
		
		cout << "Listening for client...\n";
		listenSck.Listen();
		Socket clientSck = listenSck.Accept(serverEp);
		
		cout << "Client has accepted\n";
		while (1)
		{
			cout << "Waiting to receive...\n";
			Message clientMsg = clientSck.Receive();
			if (!HandleClientMessage(clientSck, clientMsg))
				break;
		}
		return 0;
	}
	catch (NetException& ex)
	{
		cout << ex.what() << endl;
		return -1;
	}
}