#pragma once

#include "Session.h"
#include "DBHandler.h"

class IOCPServer
{
public:
	IOCPServer(const EndPoint& ep);
	virtual ~IOCPServer();

	void InitNPC();
	void Run();
	
	void Disconnect(int id);
	void AcceptNewClient(int id, SOCKET sck);

	void ProcessPackets(int id, RingBuffer& msgQueue);	
	void ProcessLoginPacket(cs_packet_login& pck, int myId);

	void SendNewPlayerInfoToNearPlayers(int target);
	void SendNearPlayersInfoToNewPlayer(int sender);

	void HandlePlayersInSight(
		const std::unordered_set<int>& sights,
		const std::unordered_set<int>& viewlist, int myId);
	void HandleDisappearedPlayers(
		const std::unordered_set<int>& sights,
		const std::unordered_set<int>& viewlist, int myId);

	void SendLoginOkPacket(int id, bool success);
	void SendPutObjectPacket(int sender, int target);
	void SendMovePacket(int sender, int target);
	void SendRemovePacket(int sender, int target);
	static void SendChatPacket(int sender, int target, char* msg);

	bool IsNPC(int id);
	bool IsNear(int a_id, int b_id);

	void HandleCompletionInfoByOperation(WSAOVERLAPPEDEX* over, int id, int bytes);
	void MoveNPC(int id, int direction);

	void MovePosition(short& x, short& y, char direction);
	static void AddTimer(int obj_id, int player_id, EventType type, int direction, int duration);
	void ActivateNPC(int id);
	void ActivatePlayerMoveEvent(int target, int player);

	void InsertIntoSectorWithLock(int id);
	void EraseFromSectorWidthLock(int id);	
	std::pair<short, short> GetSectorIndex(int id);

	int GetAvailableID();

	static int API_AddTimer(lua_State* ls);
	static int API_SendMessage(lua_State* ls);
	static int API_get_x(lua_State* ls);
	static int API_get_y(lua_State* ls);

	static void NetworkThreadFunc(IOCPServer& server);
	static void TimerThreadFunc(IOCPServer& server);

	static const int MaxThreads = 4;

private:
	Socket mListenSck;
	IOCP mIOCP;

	static std::array<std::shared_ptr<Session>, MAX_USER + MAX_NPC> gClients;
	//static std::array<std::array<std::unordered_set<int>, SECTOR_WIDTH>, SECTOR_HEIGHT> gSectors;
	static concurrency::concurrent_priority_queue<TimerEvent> gTimerQueue;

	std::mutex mSectorLock;

	std::vector<std::thread> mThreads;
	std::thread mTimerThread;
	std::atomic_bool mLoop;

	DBHandler mDBHandler;
};