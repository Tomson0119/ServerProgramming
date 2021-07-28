#pragma once

#include <WinSock2.h>
#include <string>
#include <MSWSock.h>

class EndPoint;

enum class SocketType
{
	TCP,
	UDP
};

class Socket
{
public:
	Socket();
	Socket(SocketType type, bool overlapped=false);
	Socket(SOCKET sck);
	~Socket();

	// ������ �����Ѵ�.
	void Close();

	// ������ Endpoint�� �Ҵ��Ѵ�.
	void Bind(const EndPoint& ep);

	// ���� Endpoint�� ������ ��û�Ѵ�.
	int Connect(const EndPoint& ep);

	// ���� ��û�� �� ������ ����Ѵ�.
	void Listen();

	// Ŭ���̾�Ʈ���� ������ ����ϰ� ���ο� ������ �����Ѵ�.
	int Accept(Socket& acceptSocket, std::string& errorString);

	// Overlapped I/O���� ���� ������ ����Ѵ�.
	bool AcceptOverlapped(Socket& acceptCandidateSocket, std::string& errorString);
	int UpdateAcceptContext(Socket& listenSocket);

	int Send(const std::string& data);
	int Send(const char* data, int length);

	int Receive();
	int ReceiveOverlapped();

	// ������ ���¿��� ���� ������ ��ȯ�Ѵ�. 
	EndPoint GetPeerAddr();

	// ���� �������� �����Ѵ�.
	void SetNonBlocking();

	static const int MaxReceiveBufferLength = 256;
	char mReceiveBuffer[MaxReceiveBufferLength];

private:
	// ���� ������ ���� �ʱ�ȭ�� ���̴� ����ü
	WSADATA mWData;
	
	// AcceptEx �Լ� ������
	LPFN_ACCEPTEX mAcceptEx = NULL;

public:
	// Overlapped I/O, IOCP�� �� ������ ����Ѵ�.
	// ���� overlapped I/O ���̸� true�̴�.
	// N-send, N-recv�� �����ϰ� �Ϸ��� �̷��� ���� ���� �����ؼ��� �� �ȴ�.
	bool mIsReadOverlapped = false;

private:
	// Overlapped receive or accept�� �� �� ���Ǵ� overlapped ��ü.
	// I/O �Ϸ� �������� �����Ǿ�� �Ѵ�.
	WSAOVERLAPPED mReadOverlappedStruct;

	// Overlapped ������ �ϴ� ���� recv�� flags�� ���ϴ� ���̴�.
	// I/O�� �Ϸ�� ������ �ǵ���� �� �ȴ�.
	DWORD mReadFlags = 0;

public:
	SOCKET mHandle;
};

