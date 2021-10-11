#pragma once

class ClientSocket : public Socket
{
public:
	ClientSocket(Protocol type);
	virtual ~ClientSocket();

	void SendMsg(Message& msg);
	void RecvMsg();

	void Disconnect();

	void HandleMessage(Message& msg);

public:
	static std::vector<PlayerCoord> PlayerCoords;
	int ID;

private:
	WSAOVERLAPPEDEX mSendOverlapped;
	WSAOVERLAPPEDEX mRecvOverlapped;
};