#pragma once

#include "WSAInit.h"

extern WSAInit gWSAInstance;

enum class Protocol
{
	TCP,
	UDP
};

class EndPoint;

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

	void Send();
	void Receive();


private:
	SOCKET mSocket{};
};