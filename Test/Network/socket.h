#pragma once

#ifdef _WIN32
#include <WinSock2.h>
#include <MSWSock.h>
#else
#include <sys/socket.h>
#endif

#include <string>

#ifdef __linux__
// ������������ ���� �ڵ�� int �ڷ����� ����Ѵ�.
typedef int SOCKET;
#endif

class EndPoint;

enum class SocketType
{
	TCP,
	UDP,
	OVERLAPPED_TCP,
	OVERLAPPED_UDP
};

class Socket
{
public:
	Socket();
	Socket(SocketType type);
	Socket(SOCKET sck);
	~Socket();

	// �������� ������ ����� �� �ֵ��� �ʱ�ȭ�Ѵ�.
	void InitWSA();

	// ������ �����Ѵ�.
	void Close();

	// ������ EndPoint�� �Ҵ��Ѵ�.
	void Bind(const EndPoint& ep);

	// ���� ������ ��û�Ѵ�.
	int Connect(const EndPoint& ep);

	// ������ �����⸦ ��ٸ���.
	void Listen();

	// ������ ����ϰ� ���ο� ������ �����Ѵ�.
	Socket Accept(std::string& errorText);

	// ����ŷ �������� �����Ѵ�.
	void SetNonBlocking();

private:
	// ���� ���� �� �ִ� �������� �ִ� ����
	static const int MaxReceiveLength = 1024;

	SOCKET mSck;	// ���� �ڵ�
	
	bool mNonBlocking = false;	// ���� �÷���

#ifdef _WIN32
	WSADATA mWData;

	// Overlapped receive / accept�� �� �� ����ϴ� ����ü
	// I/O �Ϸ� �������� �����Ǿ�� �Ѵ�.
	WSAOVERLAPPED mReadOverlappedStruct;
	
	// Overlapped receive�� �ϴ� ���� recv�� flags�� ���ϴ� ��.
	// I/O �Ϸ� �������� �����Ǿ�� �Ѵ�.
	DWORD mReadFlags = 0;

	// AcceptEx �Լ� ������
	LPFN_ACCEPTEX mAcceptEx;
#endif
};