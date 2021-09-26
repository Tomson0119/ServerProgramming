#pragma once

#include "WSAInit.h"

extern WSAInit gWSAInstance;

enum class Protocol
{
	TCP,
	UDP
};

class EndPoint;
class Message;

class Socket
{
public:
	Socket(Protocol type);
	Socket(SOCKET sck);
	virtual ~Socket();

	void Bind(const EndPoint& ep);
	void Listen();

	Socket Accept(EndPoint& ep);
	
	void Connect(const EndPoint& ep);

	void Send(Message& msg);
	Message Receive();

private:
	Message CreateMessage(DWORD bytes);

private:
	SOCKET mSocket{};

	static const int MaxRecvLength = 1024;
	char mReceiveBuffer[MaxRecvLength];
};