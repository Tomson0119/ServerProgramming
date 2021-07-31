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

	// �ƹ� EndPoint���� �Ҵ��� �� ���δ�.
	static EndPoint Any;

	std::string ToString();

public:
	sockaddr_in mAddr;
};