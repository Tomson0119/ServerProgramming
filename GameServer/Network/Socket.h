#pragma once

#include "WSAInit.h"
#include "WSAOverlappedEx.h"

extern WSAInit gWSAInstance;

class EndPoint;
class Message;

class Socket
{
public:
	Socket();
	Socket(SOCKET sck);
	virtual ~Socket();

	void Bind(const EndPoint& ep);
	void Listen();

	SOCKET Accept();

	void Connect(const EndPoint& ep);

	void Send(WSAOVERLAPPEDEX& overlapped);
	void Recv(WSAOVERLAPPEDEX& overlapped);

public:
	static void CALLBACK SendRoutine(DWORD err, DWORD bytes, LPWSAOVERLAPPED overlapped, DWORD flag);
	static void CALLBACK RecvRoutine(DWORD err, DWORD bytes, LPWSAOVERLAPPED overlapped, DWORD flag);

public:
	SOCKET mSocket;
	Message mReceiveBuffer;
};