#include "stdafx.h"
#include "Socket.h"
#include "EndPoint.h"
#include "Message.h"

WSAInit gWSAInstance;

Socket::Socket(Protocol type)
	: mSocket{}, mReceiveBuffer{}
{
	if (!gWSAInstance.Init())
		throw NetException();

	switch (type)
	{
	case Protocol::TCP:
		mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
		break;

	case Protocol::UDP:
		mSocket = WSASocket(AF_INET, SOCK_DGRAM, IPPROTO_UDP, 0, 0, WSA_FLAG_OVERLAPPED);
		break;
	}
}

Socket::Socket(SOCKET sck)
	: mReceiveBuffer{}
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

SOCKET Socket::Accept(EndPoint& ep)
{
	INT addr_len = (INT)sizeof(ep.address);
	SOCKET sck = WSAAccept(mSocket, reinterpret_cast<sockaddr*>(&ep.address), &addr_len, 0, 0);
	
	if (sck == INVALID_SOCKET)
		throw NetException();
	return sck;
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

void Socket::Send(WSAOVERLAPPEDEX& overlapped)
{
	DWORD bytes = 0;

	if (WSASend(mSocket,
		&overlapped.WSABuffer, 1, &bytes, 0,
		&overlapped, SendRoutine) != 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			throw NetException();
	}
}

void Socket::Recv(WSAOVERLAPPEDEX& overlapped)
{
	DWORD flag = 0;

	if (WSARecv(mSocket,
		&overlapped.WSABuffer, 1, 0, &flag,
		&overlapped, RecvRoutine) != 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			throw NetException();
	}
}