#pragma once

#include "WSAInit.h"
#include "WSAOverlappedEx.h"

extern WSAInit gWSAInstance;

class EndPoint;

class Socket
{
public:
	Socket();
	Socket(SOCKET sck);
	virtual ~Socket();

	void Bind(const EndPoint& ep);
	void Listen();
	
	void AsyncAccept(WSAOVERLAPPEDEX& accpet_ex);
	void Connect(const EndPoint& ep);

	int Send(WSAOVERLAPPEDEX& overlapped);
	int Recv(WSAOVERLAPPEDEX& overlapped);

public:
	SOCKET mSocket;
};