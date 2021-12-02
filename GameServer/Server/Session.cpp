#include "common.h"
#include "Session.h"
#include "WSAOverlappedEx.h"


Session::Session()
	: ID(-1), PosX(0), PosY(0),
	  mRecvOverlapped{}, mState(State::FREE),
	  Type(ClientType::PLAYER), 
	  Active(false), LastMoveTime(0),
	  Name{}, Lua{}
{
}

Session::~Session()
{
	if (Lua) lua_close(Lua);
}

void Session::Disconnect()
{
	mStateLock.lock();
	mState = State::FREE;
	mStateLock.unlock();
	Socket::Close();
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

void Session::InsertViewID(int id)
{
	ViewListLock.lock();
	mViewList.insert(id);
	ViewListLock.unlock();
}

void Session::EraseViewID(int id)
{
	ViewListLock.lock();
	mViewList.erase(id);
	ViewListLock.unlock();
}

bool Session::FindAndInsertViewID(int id)
{
	ViewListLock.lock();
	if (mViewList.find(id) != mViewList.end())
	{
		ViewListLock.unlock();
		return false;
	}
	mViewList.insert(id);
	ViewListLock.unlock();
	return true;
}

bool Session::FindAndEraseViewID(int id)
{
	ViewListLock.lock();
	if (mViewList.find(id) != mViewList.end())
	{
		mViewList.erase(id);
		ViewListLock.unlock();
		return true;
	}	
	ViewListLock.unlock();
	return false;
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

bool Session::IsSame(PlayerInfo& coord)
{
	return (PosX == coord.x && PosY == coord.y);
}

bool Session::IsStateWithoutLock(State state)
{
	return (mState == state);
}

bool Session::IsStateWithLock(State state)
{
	mStateLock.lock();
	bool ret = (mState == state);
	mStateLock.unlock();
	return ret;
}