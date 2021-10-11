#pragma once

class Session;

class Server
{
public:
	static std::unordered_map<int, std::unique_ptr<Session>> gClients;

public:
	Server(Protocol type, const EndPoint& ep);
	virtual ~Server();

	void Run();

	static void HandleMessage(Message& msg, int id);
	static bool CheckIfValidPosition(PlayerCoord coord);
	
private:
	PlayerCoord PeekNewPosition(int id);
	void SendClientsPosition(int id);

private:
	std::unique_ptr<Socket> mListenSck;
	EndPoint mEndPoint;

	bool mLoop;
	static const int MaxBoardSize = 8;
};