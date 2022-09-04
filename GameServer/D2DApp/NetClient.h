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
	void OnProcessKeyInput(char input);
	void SendMovePacket(char input);
	void SendAttackPacket();
	void SendChatPacket(const char* msg);

	void Disconnect();
	void SendMsg(std::byte* msg, int bytes);
	void RecvMsg();

	void HandleCompletionInfo(WSAOVERLAPPEDEX* over, int bytes);
	void ProcessPackets(WSAOVERLAPPEDEX* over, int bytes);

	static void NetworkThreadFunc(NetClient& client);

private:
	IOCP mIOCP;
	BufferQueue mMsgQueue;
	WSAOVERLAPPEDEX mRecvOverlapped;

	std::thread mSocketThread;
	std::atomic_bool mLoop;

	GraphicScene* mScene;
	ChatWindow* mChatWin;
	LogWindow* mLogWin;

	std::string mUsername;
};