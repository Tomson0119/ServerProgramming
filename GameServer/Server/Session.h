#pragma once

enum class State : char
{
	FREE,
	ACCEPT,
	INGAME
};

class Session : public Socket
{
public:
	Session();
	virtual ~Session();

	void AssignAcceptedID(int id, SOCKET sck);
	bool CompareAndChangeState(State target_state, State new_state);

	void SendLoginOkPacket();

	void SendMsg(char* msg, int bytes);
	void RecvMsg();

	bool IsSame(PlayerCoord coord);

public:
	int ID;
	PlayerCoord PlayerPos;
	RingBuffer MsgQueue;

private:
	WSAOVERLAPPEDEX mRecvOverlapped;

	std::mutex mViewListLock;
	std::unordered_set<int> mViewList;

	std::mutex mStateLock;
	State mState;
};