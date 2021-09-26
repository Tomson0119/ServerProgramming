#include "common.h"
#include <iostream>

using namespace std;

const short SERVER_PORT = 5505;

bool HandleClientMessage(Socket& client, Message& msg)
{
	switch (msg.mMsgType)
	{
	case MsgType::MSG_MOVE:
	{
		const int MaxBoardSize = 8;

		uint8_t row, col, command;
		msg.Pop(row);
		msg.Pop(col);
		msg.Pop(command);

		cout << "Message Move:\n";
		printf("Row: %d  Column: %d  Command: %d\n", row, col, command);

		if (col > 0 && command == VK_LEFT)
			col -= 1;
		else if (col < MaxBoardSize - 1 && command == VK_RIGHT)
			col += 1;
		else if (row > 0 && command == VK_DOWN)
			row -= 1;
		else if (row < MaxBoardSize - 1 && command == VK_UP)
			row += 1;

		Message newMsg(MsgType::MSG_MOVE);
		newMsg.Push((uint8_t)row);
		newMsg.Push((uint8_t)col);
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