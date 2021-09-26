#include "stdafx.h"
#include "clientSocket.h"

ClientSocket::ClientSocket(Protocol type)
	: Socket(type)
{
}

ClientSocket::~ClientSocket()
{
}

void ClientSocket::Disconnect()
{
	Send(Message(MsgType::MSG_DISCONNECT));
}
