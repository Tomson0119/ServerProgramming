#include "stdafx.h"
#include "endpoint.h"

EndPoint EndPoint::Any;

EndPoint::EndPoint()
{
	mAddr = {};
	mAddr.sin_family = AF_INET;
}

EndPoint::EndPoint(const char* ip, short port)
{
	mAddr = {};

	mAddr.sin_family = AF_INET;

	// CPU마다 데이터를 저장하는 방식(big endian, little endian)의
	// 차이가 있는데, 이는 네트워크 통신의 호환성 문제를 야기할 수 있다.
	// 따라서 이를 네트워크 바이트 순서(big endian)로 변경해주어서
	// 통신 네트워크가 원활하게 동작하도록 htons(short), htonl(long) 함수를 사용한다.
	mAddr.sin_port = htons((u_short)port);

	// IP 주소를 바이너리로 변환하여 저장한다.
	inet_pton(AF_INET, ip, &mAddr.sin_addr);
}

EndPoint::EndPoint(short port)
{
	mAddr = {};
	mAddr.sin_family = AF_INET;
	mAddr.sin_port = htons((u_short)port);
	mAddr.sin_addr.s_addr = htonl(INADDR_ANY);
}

EndPoint::~EndPoint()
{
}

std::string EndPoint::ToString()
{
	char address[1000];
	address[0] = 0;
	// 바이너리 주소 값을 IPv4 형식에 맞춰 문자열로 변환한다.
	inet_ntop(AF_INET, &mAddr.sin_addr, address, sizeof(address) - 1);

	char finalString[1010];

#ifdef _WIN32
	// IP 주소와 포트 번호를 string:int 형식으로 묶는다.
	sprintf_s(finalString, "%s:%d", address, mAddr.sin_port);
#else
	sprintf(finalString, "%s:%hu", address, (u_short)mAddr.sin_port);
#endif

	return finalString;
}
