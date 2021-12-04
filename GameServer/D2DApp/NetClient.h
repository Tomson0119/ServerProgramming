#pragma once

class GraphicScene;
class ChatWindow;
class LogWindow;

class NetClient : public Socket
{
public:
	NetClient();
	virtual ~NetClient();

	void SetInterfaces(GraphicScene* scene, ChatWindow* chatWin, LogWindow* logWin);
	void Start(const std::string& name);

	void SendLoginPacket(const char* name);
	void SendMovePacket(char input);
	void SendChatPacket(const char* msg);

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
	ChatWindow* mChatWin;
	LogWindow* mLogWin;

	std::string mUsername;
};