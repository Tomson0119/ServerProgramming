#pragma once

enum class State : char
{
	FREE,
	ACCEPT,
	INGAME,
	SLEEP
};

enum class ClientType : char
{
	PLAYER,
	NPC
};

class Session : public Socket
{
public:
	Session();
	virtual ~Session();

	void Disconnect();

	void AssignAcceptedID(int id, SOCKET sck);
	bool CompareAndChangeState(State target_state, State new_state);

	void InsertViewID(int id);
	bool FindAndInsertViewID(int id);
	bool FindAndEraseViewID(int id);
	bool EraseViewID(int id);

	void SendMsg(char* msg, int bytes);
	void RecvMsg();
	//void PushMsg(uchar* msg, int bytes);

	bool IsSame(PlayerInfo& coord);
	bool IsStateWithLock(State state);
	bool IsStateWithoutLock(State state);

	void InitState(State state) { mState = state; }

	const std::unordered_set<int>& GetViewList() const { return mViewList; }

public:
	int ID;
	PlayerInfo Info;
	char Name[MAX_NAME_SIZE];	

	ClientType Type;
	int LastMoveTime;
	std::atomic_bool Active;

	std::mutex ViewListLock;
	std::mutex MsgQueLock;
	//RingBuffer MsgQueue;

	lua_State* Lua;

private:
	WSAOVERLAPPEDEX mRecvOverlapped;	
	std::unordered_set<int> mViewList;

	std::mutex mStateLock;
	State mState;
};