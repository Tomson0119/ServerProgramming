#pragma once

#include <WS2tcpip.h>

class EndPoint
{
public:
	EndPoint();
	EndPoint(const char* address, int port);
	~EndPoint();

	// EndPoint�� �������� �ִ� ��������
	static EndPoint Any;

	// IPv4 �ּҿ� ��Ʈ ��ȣ�� ��� �ִ� ����ü 
	sockaddr_in mIPv4Endpoint;

	// Endpoint�� "ip:port" �������� ��ȯ�ϴ� �Լ�
	std::string ToString();
};