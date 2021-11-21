#include "common.h"
#include "Session.h"
#include "WSAOverlappedEx.h"


Session::Session()
	: ID(-1), PlayerPos{0,0}, 
	  mRecvOverlapped{}, mState(State::FREE)
{
}

Session::~Session()
{
}

void Session::AssignAcceptedID(int id, SOCKET sck)
{
	ID = id;
	mSocket = sck;
}

bool Session::CompareAndChangeState(State target_state, State new_state)
{
	bool ret = false;
	mStateLock.lock();
	if (mState == target_state)
	{
		mState = new_state;
		ret = true;
	}
	mStateLock.unlock();
	return ret;
}

void Session::SendLoginOkPacket()
{
}

void Session::SendMsg(char* msg, int bytes)
{
	WSAOVERLAPPEDEX* send_over = new WSAOVERLAPPEDEX(OP::SEND, msg, bytes);
	Send(*send_over);
}

void Session::RecvMsg()
{
	mRecvOverlapped.Reset(OP::RECV);
	Recv(mRecvOverlapped);
}

bool Session::IsSame(PlayerCoord coord)
{
	return (PlayerPos.Col == coord.Col && PlayerPos.Row == coord.Row);
}
