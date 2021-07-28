#pragma once

#include <WS2tcpip.h>

class EndPoint
{
public:
	EndPoint();
	EndPoint(const char* address, int port);
	~EndPoint();

	// EndPoint를 무작위로 넣는 전역변수
	static EndPoint Any;

	// IPv4 주소와 포트 번호를 담고 있는 구조체 
	sockaddr_in mIPv4Endpoint;

	// Endpoint를 "ip:port" 형식으로 변환하는 함수
	std::string ToString();
};