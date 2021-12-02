#pragma once

class GraphicScene;

class NetClient : public Socket
{
public:
	NetClient();
	virtual ~NetClient();

	void Start(GraphicScene* scene, const char* name);

	void SendLoginPacket(const char* name);
	void SendMovePacket(char input);

	void Disconnect();

	static void Update(NetClient& client);

	void SendMsg(char* msg, int bytes);
	void RecvMsg();
	void ProcessPackets();

private:
	IOCP mIOCP;
	RingBuffer mMsgQueue;
	WSAOVERLAPPEDEX mRecvOverlapped;

	std::thread mSocketThread;
	std::atomic_bool mLoop;

	GraphicScene* mScene;
};