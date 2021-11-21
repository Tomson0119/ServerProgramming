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
	void HandleMessage(unsigned char* msg);

	int GetTotalPlayers() const { return (int)PlayerCoords.size(); }

public:
	std::unordered_map<int, PlayerCoord> PlayerCoords;
	int ID;
	int PrevSize;
	bool Dirty;

private:
	IOCP mIOCP;
	WSAOVERLAPPEDEX mRecvOverlapped;
	bool mLoop;

	std::thread mSocketThread;
};