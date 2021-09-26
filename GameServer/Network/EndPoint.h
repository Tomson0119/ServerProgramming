#pragma once

class EndPoint
{
public:
	EndPoint() = default;
	EndPoint(const std::string& ip, short port);
	~EndPoint();

	static EndPoint Any(short port);

	sockaddr_in address;
};