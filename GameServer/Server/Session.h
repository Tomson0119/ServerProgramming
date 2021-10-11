#pragma once

class Session : public Socket
{
public:
	Session() = default;
	Session(int id, SOCKET s);
	~Session();

	void SendMsg(Message& msg);
	void RecvMsg();

	bool IsSame(PlayerCoord coord);

public:
	int ID;
	PlayerCoord PlayerPos{ 0,0 };

private:
	WSAOVERLAPPEDEX mRecvOverlapped;
};