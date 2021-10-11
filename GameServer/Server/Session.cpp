#include "common.h"
#include "Session.h"

Session::Session(int id, SOCKET s)
	: ID(id), Socket(s)
{
	int tcp_opt = 1;
	setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&tcp_opt), sizeof(int));
}

Session::~Session()
{
}

void Session::SendMsg(Message& msg)
{
	char* data = reinterpret_cast<char*>(&msg);
	size_t size = msg.GetPacketSize();

	WSAOVERLAPPEDEX* sendOverEx = new WSAOVERLAPPEDEX(size, data, this);
	Send(*sendOverEx);
}

void Session::RecvMsg()
{
	ZeroMemory(&mRecvOverlapped, sizeof(mRecvOverlapped));
	mRecvOverlapped.WSABuffer.buf = reinterpret_cast<char*>(&mReceiveBuffer);
	mRecvOverlapped.WSABuffer.len = MaxRecvSize;
	mRecvOverlapped.Caller = this;

	Recv(mRecvOverlapped);
}

bool Session::IsSame(PlayerCoord coord)
{
	return (PlayerPos.Col == coord.Col && PlayerPos.Row == coord.Row);
}
