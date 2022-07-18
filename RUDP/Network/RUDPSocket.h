#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "MSWSock.lib")

#include <string>
#include <cstddef>
#include <vector>
#include <thread>
#include <atomic>
#include <queue>
#include <map>
#include <mutex>

struct message
{
	std::string peer;
	std::vector<std::byte> packet;
};

class RUDPSocket
{
public:
	enum class SckType
	{
		UDP
	};

public:
	RUDPSocket(const SckType& type);
	virtual ~RUDPSocket();

	void connect();
	void bind(u_short port);
	
	void printEndpoint();

	void pushToSendBuffer(std::byte* packet, size_t pckSize, sockaddr_in& hostEp);

	void sendTo(std::byte* packet, size_t pckSize, sockaddr_in& hostEp);
	void recvFrom(std::byte* buf, size_t bufSize, sockaddr_in& senderEp);

private:
	bool getFrontMsg(message& msg);

public:
	static void printIPAndPort(const sockaddr_in& addr);
	static std::string getPeerKey(const sockaddr_in& addr);
	static sockaddr_in stringToEp(const std::string& peer);

	static void sendThreadFunc(RUDPSocket& sck);

private:
	static const int BUF_SIZE = 256;
	std::byte recvBuffer[BUF_SIZE];

	SOCKET sckHandle;
	sockaddr_in hostEp;

	std::thread sendThread;
	std::atomic_bool loop;

	std::mutex mainQueMut;
	std::queue<message> mainSendQueue;
};