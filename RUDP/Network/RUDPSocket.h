#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "MSWSock.lib")

#include <string>
#include <cstddef>

class RUDPSocket
{
public:
	enum class SckType
	{
		TCP,
		UDP
	};

public:
	RUDPSocket(const SckType& type);
	virtual ~RUDPSocket();

	void bind(u_short port);
	void printEndpoint();

	void setHostEndpoint(const std::string& ip, const u_short port);
	void sendTo(std::byte* packet, size_t pckSize);
	void recvFrom(std::byte* buf, size_t bufSize);

private:
	SOCKET sckHandle;
	sockaddr_in hostEp;
};