#include "stdafx.h"
#include "Socket.h"
#include "EndPoint.h"

WSAInit gWSAInstance;

Socket::Socket(Protocol type)
{
	if (!gWSAInstance.Init())
		throw NetException();
	
	switch (type)
	{
	case Protocol::TCP:
		mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, 0);
		break;

	case Protocol::UDP:
		mSocket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, 0, 0, 0);
		break;
	}
}

Socket::Socket(SOCKET sck)
{
	mSocket = sck;
}

Socket::~Socket()
{
	closesocket(mSocket);
}

void Socket::Bind(const EndPoint& ep)
{
	if (bind(mSocket, reinterpret_cast<const sockaddr*>(&ep.address), sizeof(ep.address)) != 0)
		throw NetException();
}

void Socket::Listen()
{
	if (listen(mSocket, SOMAXCONN) != 0)
		throw NetException();
}

Socket Socket::Accept(EndPoint& ep)
{
	INT addr_len = (INT)sizeof(ep.address);
	SOCKET sck = WSAAccept(mSocket, reinterpret_cast<sockaddr*>(&ep.address), &addr_len, 0, 0);
	
	if (sck == INVALID_SOCKET)
		throw NetException();
	return Socket(sck);
}

void Socket::Connect(const EndPoint& ep)
{
	if (WSAConnect(mSocket,
		reinterpret_cast<const sockaddr*>(&ep.address), 
		sizeof(ep.address),	0, 0, 0, 0) != 0)
	{
		throw NetException();
	}
}

void Socket::Send()
{
	DWORD bytes;
	WSABUF buf{};
	buf.buf;
	buf.len;

	if (WSASend(mSocket, &buf, 1, &bytes, 0, 0, 0) != 0)
		throw NetException();
}

void Socket::Receive()
{
	DWORD bytes;
	DWORD flag = 0;
	WSABUF buf{};
	buf.buf;
	buf.len;
	WSARecv(mSocket, &buf, 1, &bytes, &flag, 0, 0);
}
