#pragma once

class ClientSocket : public Socket
{
public:
	ClientSocket();
	virtual ~ClientSocket();

	void CreateIOCP();
	void AssignThread();

	void SendLoginPacket();
	void SendMovePacket(char input);

	void Disconnect();

	static void Update(ClientSocket& client);

	void SendMsg(char* msg, int bytes);
	void RecvMsg();
	void ProcessPackets();

	int GetTotalPlayers() const { return (int)PlayerInfos.size(); }

public:
	std::mutex PlayerInfoLock;
	std::unordered_map<int, PlayerInfo> PlayerInfos;

	int ID;
	int PrevSize;
	bool Dirty;

private:
	IOCP mIOCP;
	RingBuffer mMsgQueue;
	WSAOVERLAPPEDEX mRecvOverlapped;
	std::thread mSocketThread;
	
	bool mLoop;

};