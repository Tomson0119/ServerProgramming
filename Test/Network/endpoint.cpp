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

	// ���ڿ� address�� binary�� ��ȯ �� sin_addr�� ����ִ´�.
	inet_pton(AF_INET, address, &mIPv4Endpoint.sin_addr);

	// CPU���� �����͸� �����ϴ� ���(big endian, little endian)��
	// ���̰� �ִµ�, �̴� ��Ʈ��ũ ����� ȣȯ�� ������ �߱��� �� �ִ�.
	// ���� �̸� ��Ʈ��ũ ����Ʈ ����(big endian)�� �������־
	// ��� ��Ʈ��ũ�� ��Ȱ�ϰ� �����ϵ��� htons(short), htonl(long) �Լ��� ����Ѵ�.
	mIPv4Endpoint.sin_port = htons((u_short)port);
}

EndPoint::~EndPoint()
{
}

std::string EndPoint::ToString()
{
	char address[1000];
	address[0] = 0;
	
	// sin_addr ���̳ʸ� �ּҸ� ���ڿ��� ��ȯ�Ѵ�.
	inet_ntop(AF_INET, &mIPv4Endpoint.sin_addr, address, sizeof(address) - 1);
	
	char str[1000];
	sprintf_s(str, "%s:%d", address, htons(mIPv4Endpoint.sin_port));
	
	return str;
}
