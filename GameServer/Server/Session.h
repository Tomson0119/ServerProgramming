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
	void EraseViewID(int id);

	void SendMsg(char* msg, int bytes);
	void RecvMsg();

	bool IsSame(int x, int y);
	bool IsStateWithLock(State state);
	bool IsStateWithoutLock(State state);

	void InitState(State state) { mState = state; }

	const std::unordered_set<int>& GetViewList() const { return mViewList; }

public:
	void DecreaseHP(int amount) { Info.hp -= amount; }
	bool IsDead() const { return (Info.hp <= 0); }
	void Revive();
	int IncreaseEXP(int opLevel);
	void IncreaseLevelWithCondition();

	void SetAttackDuration(std::chrono::milliseconds time);
	bool IsAttackTimeOut() const;

public:
	int ID;
	PlayerInfo Info;
	int AttackPower;

	ClientType Type;
	int LastMoveTime;
	std::atomic_bool Active;

	std::mutex ViewListLock;
	std::mutex MsgQueLock;

	lua_State* Lua;

private:
	WSAOVERLAPPEDEX mRecvOverlapped;	
	std::unordered_set<int> mViewList;

	std::mutex mStateLock;
	std::atomic<State> mState;

	std::chrono::system_clock::time_point mAttackedTime;
	std::chrono::milliseconds mAttackDuration;
};