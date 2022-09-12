#pragma once

#include "Session.h"
#include "DBHandler.h"
#include "Timer.h"
#include "IOCP.h"

class SectorManager;

class IOCPServer
{
public:
	IOCPServer(const EndPoint& ep);
	virtual ~IOCPServer();

	void InitNPC();
	void Run();
	
	void Disconnect(int id);
	void AcceptNewClient(int id, SOCKET sck);

	void ProcessPackets(WSAOVERLAPPEDEX* over, int id, int bytes);	
	void ProcessLoginPacket(cs_packet_login* pck, int myId);
	void ProcessAttackPacket(int id, const std::unordered_set<int>& viewlist);

	void HandlePlayersInSight(
		const std::unordered_set<int>& sights,
		const std::unordered_set<int>& viewlist, int myId);
	void HandleDisappearedPlayers(
		const std::unordered_set<int>& sights,
		const std::unordered_set<int>& viewlist, int myId);

	void PostNPCMoveEvent(int objectId, int targetId, int direction);

private:
	void SendNearPlayersInfo(int target);
	void SendLoginOkPacket(int id);
	void SendLoginFailPacket(int id, char reason);
	void SendPutObjectPacket(int sender, int target);
	void SendMovePacket(int sender, int target);
	void SendRemovePacket(int sender, int target);
	void SendStatusChangePacket(int sender);
	void SendBattleResultPacket(int sender, int target, int val, char type);

	static void SendChatPacket(int sender, int target, char* msg);

public:
	void HandleCompletionInfo(WSAOVERLAPPEDEX* over, int id, int bytes);
	void MoveNPC(int id, int direction);
	void HandleDeadNPC(int id);
	void HandleRevivedPlayer(int id);
	
	void ActivateNPC(int id);
	void ActivatePlayerMoveEvent(int target, int player);

	int GetAvailableID();

private:
	static int API_AddTimer(lua_State* ls);
	static int API_SendMessage(lua_State* ls);
	static int API_get_x(lua_State* ls);
	static int API_get_y(lua_State* ls);

	static void AddTimer(int obj_id, int player_id, EventType type, int direction, int duration);
	static void NetworkThreadFunc(IOCPServer& server);
	static void SignalHandler(int signal);
	
	static const int MaxThreads = 6;

private:
	Socket mListenSck;

	std::unique_ptr<SectorManager> mSectorManager;

	std::vector<std::thread> mThreads;
	std::atomic_bool mLoop;

	DBHandler mDBHandler;
	
	static Timer gTimer;
	static IOCP gIOCP;

	static std::array<std::shared_ptr<Session>, MAX_USER + MAX_NPC> gClients;
};