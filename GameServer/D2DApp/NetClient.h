#pragma once

class GraphicScene;

class NetClient : public Socket
{
public:
	NetClient();
	virtual ~NetClient();

	void Start(GraphicScene* scene);

	void SendLoginPacket();
	void SendMovePacket(char input);

	void Disconnect();

	static void Update(NetClient& client);

	void SendMsg(char* msg, int bytes);
	void RecvMsg();
	void ProcessPackets();

	int GetTotalPlayers() const { return (int)PlayerInfos.size(); }

public:
	std::mutex PlayerCoordLock;
	std::unordered_map<int, PlayerInfo> PlayerInfos;

	int ID;
	int PrevSize;
	bool Dirty;

private:
	IOCP mIOCP;
	RingBuffer mMsgQueue;
	WSAOVERLAPPEDEX mRecvOverlapped;

	std::thread mSocketThread;

	GraphicScene* mScene;

	std::atomic_bool mLoop;
};