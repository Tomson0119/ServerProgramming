#include "common.h"
#include "Session.h"
#include "WSAOverlappedEx.h"


Session::Session()
	: ID(-1), Info{}, Lua{},
	  mRecvOverlapped{}, mState(State::FREE),
	  Type(ClientType::PLAYER), 
	  Active(false), LastMoveTime(0), AttackPower(0)
{
	SetAttackDuration(0ms);
}

Session::~Session()
{
	if (Lua) lua_close(Lua);
}

void Session::Disconnect()
{
	mState = State::FREE;
	Socket::Close();
}

void Session::InitLuaEngine(const std::string& file)
{
	if (ID < 0)
	{
		std::cout << "Cannot set npc uid: [uid is negative]\n";
		return;
	}
	Lua = luaL_newstate();
	luaL_openlibs(Lua);
	int err = luaL_loadfile(Lua, file.c_str());
	if (err)
	{
		std::cout << "Lua Error: " << lua_tostring(Lua, -1) << std::endl;
		return;
	}
	lua_pcall(Lua, 0, 0, 0);

	lua_getglobal(Lua, "set_uid");
	lua_pushnumber(Lua, ID);
	err = lua_pcall(Lua, 1, 0, 0);
	if (err)
	{
		std::cout << "Lua Error: " << lua_tostring(Lua, -1) << std::endl;
		return;
	}
	lua_pop(Lua, 1);
}

void Session::RegisterLuaFunc(const std::string& funcName, lua_CFunction funcPtr)
{
	if (Lua == nullptr) return;
	lua_register(Lua, funcName.c_str(), funcPtr);
}

void Session::ExecuteLuaFunc(const std::string& funcName, int playerId)
{
	if (Lua == nullptr)
		return;
	
	std::unique_lock<std::mutex> lock{ LuaMut };
	lua_getglobal(Lua, funcName.c_str());
	lua_pushnumber(Lua, playerId);
	int err = lua_pcall(Lua, 1, 0, 0);
	if (err)
	{
		std::cout << "Lua Error: " << lua_tostring(Lua, -1) << std::endl;
		lua_pop(Lua, 1);
	}
}

void Session::AssignAcceptedID(int id, SOCKET sck)
{
	ID = id;
	mSocket = sck;
}

bool Session::CompareAndChangeState(State target_state, State new_state)
{
	return (mState.compare_exchange_strong(target_state, new_state));
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

void Session::SendMsg(std::byte* msg, int bytes)
{
	WSAOVERLAPPEDEX* send_over = new WSAOVERLAPPEDEX(OP::SEND, msg, bytes);
	Send(*send_over);
}

void Session::RecvMsg()
{
	mRecvOverlapped.Reset(OP::RECV);
	Recv(mRecvOverlapped);
}

bool Session::IsSamePosition(short x, short y)
{
	return (Info.x == x && Info.y == y);
}

bool Session::IsState(const State& state)
{
	return (mState == state);
}

std::unordered_set<int> Session::GetViewList()
{
	std::unique_lock<std::mutex> lock{ ViewListLock };
	return mViewList;
}

void Session::Revive()
{
	Info.hp.store(Info.max_hp);
	mState = State::INGAME;
}

int Session::IncreaseEXP(int opLevel)
{
	Info.exp += opLevel * opLevel * 2;
	IncreaseLevelWithCondition();
	return opLevel * opLevel * 2;
}

void Session::IncreaseLevelWithCondition()
{
	while (true) {
		int expNeeded = 100 * (int)pow(2, Info.level - 1);
		if (expNeeded <= Info.exp)
		{
			Info.exp -= expNeeded;
			Info.level += 1;
			Info.max_hp = 150 * Info.level;
			Info.hp.store(Info.max_hp);
			AttackPower = Info.level * 5;
		}
		else break;
	}
}

void Session::SetAttackDuration(std::chrono::milliseconds time)
{
	mAttackedTime = std::chrono::system_clock::now();
	mAttackDuration = time;
}

bool Session::IsAttackTimeOut() const
{
	return (mAttackedTime + mAttackDuration < std::chrono::system_clock::now());
}