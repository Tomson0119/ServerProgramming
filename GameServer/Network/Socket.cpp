#include "stdafx.h"
#include "Socket.h"
#include "EndPoint.h"
#include "Message.h"
#include <iostream>

WSAInit gWSAInstance;

Socket::Socket()
	: mReceiveBuffer{}
{
	if (!gWSAInstance.Init())
		throw NetException("WSAData Initialize failed");

	mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
}

Socket::Socket(SOCKET sck)
	: mReceiveBuffer{}
{
	mSocket = sck;
}

Socket::~Socket()
{
	if(mSocket)
		closesocket(mSocket);
}

void Socket::Bind(const EndPoint& ep)
{
	if (bind(mSocket, reinterpret_cast<const sockaddr*>(&ep.address), sizeof(ep.address)) != 0)
		throw NetException("Bind failed");
}

void Socket::Listen()
{
	if (listen(mSocket, SOMAXCONN) != 0)
		throw NetException("Listen failed");
}

SOCKET Socket::Accept()
{
	SOCKET sck = WSAAccept(mSocket, NULL, 0, 0, 0);
	
	if (sck == INVALID_SOCKET)
		throw NetException("Accept failed");
	return sck;
}

void Socket::Connect(const EndPoint& ep)
{
	if (WSAConnect(mSocket,
		reinterpret_cast<const sockaddr*>(&ep.address), 
		sizeof(ep.address),	0, 0, 0, 0) != 0)
	{
		throw NetException("Connect failed");
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
			throw NetException("Send failed");
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
			throw NetException("Recv failed");
	}
}