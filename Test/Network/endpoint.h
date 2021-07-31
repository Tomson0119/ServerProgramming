#pragma once

#ifdef _WIN32
#include <WS2tcpip.h>
#else
#include <netinet/in.h>
#endif

#include <string>


class EndPoint
{
public:
	EndPoint();
	EndPoint(const char* ip, short port);
	EndPoint(short port);
	~EndPoint();

	// 아무 EndPoint에나 할당할 때 쓰인다.
	static EndPoint Any;

	std::string ToString();

public:
	sockaddr_in mAddr;
};