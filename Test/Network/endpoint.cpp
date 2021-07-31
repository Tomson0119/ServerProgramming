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

	// CPU���� �����͸� �����ϴ� ���(big endian, little endian)��
	// ���̰� �ִµ�, �̴� ��Ʈ��ũ ����� ȣȯ�� ������ �߱��� �� �ִ�.
	// ���� �̸� ��Ʈ��ũ ����Ʈ ����(big endian)�� �������־
	// ��� ��Ʈ��ũ�� ��Ȱ�ϰ� �����ϵ��� htons(short), htonl(long) �Լ��� ����Ѵ�.
	mAddr.sin_port = htons((u_short)port);

	// IP �ּҸ� ���̳ʸ��� ��ȯ�Ͽ� �����Ѵ�.
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
	// ���̳ʸ� �ּ� ���� IPv4 ���Ŀ� ���� ���ڿ��� ��ȯ�Ѵ�.
	inet_ntop(AF_INET, &mAddr.sin_addr, address, sizeof(address) - 1);

	char finalString[1010];

#ifdef _WIN32
	// IP �ּҿ� ��Ʈ ��ȣ�� string:int �������� ���´�.
	sprintf_s(finalString, "%s:%d", address, mAddr.sin_port);
#else
	sprintf(finalString, "%s:%hu", address, (u_short)mAddr.sin_port);
#endif

	return finalString;
}
