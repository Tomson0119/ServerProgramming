#include "stdafx.h"
#include "Socket.h"
#include "EndPoint.h"
#include "WSAOverlappedEx.h"

#include <iostream>

WSAInit gWSAInstance;

Socket::Socket()
	: mSocket{}
{
	if (!gWSAInstance.Init())
		throw NetException("WSAData Initialize failed");
}

Socket::Socket(SOCKET sck)
{
	mSocket = sck;
}

Socket::~Socket()
{
	Close();
}

void Socket::Close()
{
	if (mSocket)
		closesocket(mSocket);
}

void Socket::Init()
{
	mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (mSocket == INVALID_SOCKET)
		throw NetException("Socket creation failed");
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

void Socket::AsyncAccept(WSAOVERLAPPEDEX& accept_ex)
{
	SOCKET sck = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, 0, 0, WSA_FLAG_OVERLAPPED);
	if (sck == INVALID_SOCKET)
		throw NetException("Socket is invalid");

	accept_ex.Reset(OP::ACCEPT, reinterpret_cast<char*>(&sck), sizeof(sck));
	
	if (AcceptEx(mSocket, sck, accept_ex.NetBuffer + sizeof(SOCKET), 0, sizeof(sockaddr_in) + 16,
		sizeof(sockaddr_in) + 16, NULL, &accept_ex.Overlapped) == FALSE)
	{
		if(WSAGetLastError() != WSA_IO_PENDING)
			throw NetException("AcceptEx failed");
	}
}

bool Socket::Connect(const EndPoint& ep)
{
	if (WSAConnect(mSocket,
		reinterpret_cast<const sockaddr*>(&ep.address), 
		sizeof(ep.address),	0, 0, 0, 0) != 0)
	{
		return false;
	}
	return true;
}

int Socket::Send(WSAOVERLAPPEDEX& overlapped)
{
	DWORD bytes = 0;

	if (WSASend(mSocket,
		&overlapped.WSABuffer, 1, &bytes, 0,
		&overlapped.Overlapped, NULL) != 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			return 0;
	}
	return (int)bytes;
}

int Socket::Recv(WSAOVERLAPPEDEX& overlapped)
{
	DWORD flag = 0;
	DWORD bytes = 0;

	if (WSARecv(mSocket,
		&overlapped.WSABuffer, 1, &bytes, &flag,
		&overlapped.Overlapped, NULL) != 0)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
			return 0;
	}
	return (int)bytes;
}