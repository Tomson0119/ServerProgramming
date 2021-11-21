#pragma once

#include <thread>
#include <atomic>

#include "Session.h"

class IOCPServer
{
public:
	IOCPServer(const EndPoint& ep);
	virtual ~IOCPServer();

	void Run();
	
	void Disconnect(int id);
	void AcceptNewClient(int id, SOCKET sck);
	void ProcessPackets(int id);
	
	void ProcessLoginPacket(cs_packet_login& pck, int myId);

	int GetAvailableID();

	static void HandleMessage(RingBuffer& msg, int id);
	static bool CheckIfValidPosition(PlayerCoord coord);
	
private:
	static void RunThread(IOCPServer& server);

	PlayerCoord PeekNewPosition(int id);
	void SendClientsPosition(int id);

private:
	static const int MaxPlayers = 10;
	static const int MaxThreads = 1;
	static const int MaxBoardSize = 8;

private:
	Socket mListenSck;
	IOCP mIOCP;

	std::array<Session, MaxPlayers> mClients;
	std::vector<std::thread> mThreads;

	std::atomic_bool mLoop;
};