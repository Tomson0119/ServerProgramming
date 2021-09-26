#include "stdafx.h"
#include "EndPoint.h"

EndPoint::EndPoint(const std::string& ip, short port)
{
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &address.sin_addr);
}

EndPoint::~EndPoint()
{
}

EndPoint EndPoint::Any(short port)
{
	EndPoint ep;
	ep.address.sin_family = AF_INET;
	ep.address.sin_port = htons(port);
	ep.address.sin_addr.s_addr = htonl(INADDR_ANY);
	return ep;
}
