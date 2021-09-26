#include "common.h"
#include <iostream>

using namespace std;

const short SERVER_PORT = 5505;

bool HandleClientMessage(Message& msg)
{
	cout << msg.size << endl;
	if (msg.size == 0)
		return false;

	switch (msg.mMsgType)
	{
	case MsgType::MSG_MOVE:
		uint8_t row, col, command;
		msg.Pop(row);
		msg.Pop(col);
		msg.Pop(command);

		printf("0x%x 0x%x 0x%x\n", row, col, command);
		break;
	}
	return true;
}

int main()
{
	try {
		Socket listenSck(Protocol::TCP);
		EndPoint serverEp = EndPoint::Any(SERVER_PORT);
		listenSck.Bind(serverEp);
		listenSck.Listen();
		Socket clientSck = listenSck.Accept(serverEp);
		
		cout << "Client has accepted\n";
		while (1)
		{
			cout << "Waiting to receive...\n";
			Message clientMsg = clientSck.Receive();
			if (!HandleClientMessage(clientMsg))
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