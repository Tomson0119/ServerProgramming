#include "stdafx.h"
#include "endpoint.h"

EndPoint EndPoint::Any;

EndPoint::EndPoint()
{
	memset(&mIPv4Endpoint, 0, sizeof(mIPv4Endpoint));
	mIPv4Endpoint.sin_family = AF_INET;
}

EndPoint::EndPoint(const char* address, int port)
{
	memset(&mIPv4Endpoint, 0, sizeof(mIPv4Endpoint));
	mIPv4Endpoint.sin_family = AF_INET;

	// 문자열 address를 binary로 변환 후 sin_addr에 집어넣는다.
	inet_pton(AF_INET, address, &mIPv4Endpoint.sin_addr);

	// CPU마다 데이터를 저장하는 방식(big endian, little endian)의
	// 차이가 있는데, 이는 네트워크 통신의 호환성 문제를 야기할 수 있다.
	// 따라서 이를 네트워크 바이트 순서(big endian)로 변경해주어서
	// 통신 네트워크가 원활하게 동작하도록 htons(short), htonl(long) 함수를 사용한다.
	mIPv4Endpoint.sin_port = htons((u_short)port);
}

EndPoint::~EndPoint()
{
}

std::string EndPoint::ToString()
{
	char address[1000];
	address[0] = 0;
	
	// sin_addr 바이너리 주소를 문자열로 변환한다.
	inet_ntop(AF_INET, &mIPv4Endpoint.sin_addr, address, sizeof(address) - 1);
	
	char str[1000];
	sprintf_s(str, "%s:%d", address, htons(mIPv4Endpoint.sin_port));
	
	return str;
}
