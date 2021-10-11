#include "stdafx.h"
#include "clientSocket.h"

std::vector<PlayerCoord> ClientSocket::PlayerCoords;

void Socket::SendRoutine(DWORD err, DWORD bytes, LPWSAOVERLAPPED overlapped, DWORD flag)
{
}

void Socket::RecvRoutine(DWORD err, DWORD bytes, LPWSAOVERLAPPED overlapped, DWORD flag)
{
	WSAOVERLAPPEDEX* recvOverEx = reinterpret_cast<WSAOVERLAPPEDEX*>(overlapped);
	ClientSocket* caller = reinterpret_cast<ClientSocket*>(recvOverEx->Caller);
	caller->HandleMessage(caller->mReceiveBuffer);

	caller->RecvMsg();
}

ClientSocket::ClientSocket(Protocol type)
	: Socket(type), ID(-1)
{
	int tcp_opt = 1;
	setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&tcp_opt), sizeof(int));
}

ClientSocket::~ClientSocket()
{
}

void ClientSocket::SendMsg(Message& msg)
{
	ZeroMemory(&mSendOverlapped, sizeof(mSendOverlapped));
	mSendOverlapped.WSABuffer.buf = reinterpret_cast<char*>(&msg);
	mSendOverlapped.WSABuffer.len = (ULONG)msg.GetPacketSize();
	mSendOverlapped.Caller = this;

	Send(mSendOverlapped);
}

void ClientSocket::RecvMsg()
{
	ZeroMemory(&mRecvOverlapped, sizeof(mRecvOverlapped));
	mRecvOverlapped.WSABuffer.buf = reinterpret_cast<char*>(&mReceiveBuffer);
	mRecvOverlapped.WSABuffer.len = MaxRecvSize;
	mRecvOverlapped.Caller = this;

	Recv(mRecvOverlapped);
}

void ClientSocket::Disconnect()
{
	
}

void ClientSocket::HandleMessage(Message& msg)
{
	switch (msg.Header.msg_type)
	{
	case MsgType::MSG_ACCEPT:
	{
		ID = msg.Body[0];
		for (int i = 0; i < msg.Header.size - 1; i += 2)
		{
			uint8_t x = msg.Body[i + 1];
			uint8_t y = msg.Body[i + 2];
			PlayerCoords.push_back({ x,y });
		}
		break;
	}
	case MsgType::MSG_JOIN:
	{
		uint8_t x = msg.Body[0];
		uint8_t y = msg.Body[1];
		PlayerCoords.push_back({ x,y });
		break;
	}
	case MsgType::MSG_MOVE:
	{
		int id = (int)msg.Body[0];
		uint8_t x = msg.Body[1];
		uint8_t y = msg.Body[2];
		PlayerCoords[id] = { x,y };
		break;
	}}
}
