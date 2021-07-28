#include "stdafx.h"
#include "socket.h"
#include "endpoint.h"
#include "exception.h"

Socket::Socket()
{
	mWData = {};
	mHandle = -1;
	memset(mReceiveBuffer, 0, sizeof(mReceiveBuffer));
	ZeroMemory(&mReadOverlappedStruct, sizeof(mReadOverlappedStruct));
}

Socket::Socket(SocketType type, bool overlapped)
{
	// ������ �������α׷��ֿ��� ���� ���� ȣ���ϴ� �Լ��̴�.
	int result = WSAStartup(MAKEWORD(2, 2), &mWData); // 2.2 ����
	if (result != 0) throw Exception("Failed to initialize WSA");

	if (type == SocketType::TCP)
	{
		if (overlapped)
		{
			// ������ TCP ������ �����.
			mHandle = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		}
		else
		{
			// ���� �ڵ��� �����Ѵ�. (AF_INET : IPv4, AF_INET : IPv6)
			// ��Ʈ�� �����͸� �ٷ�� TCP ���������� �����Ѵ�.
			mHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		}
		
	}
	else
	{
		if (overlapped)
		{
			// ������ UDP ������ �����.
			mHandle = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		}
		else
		{
			// �����ͱ׷��� �ٷ�� UDP ���������� �����Ѵ�.
			mHandle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		}
	}
	if (mHandle == INVALID_SOCKET)
		throw Exception("Failed to create a socket");
	memset(mReceiveBuffer, 0, sizeof(mReceiveBuffer));
	ZeroMemory(&mReadOverlappedStruct, sizeof(mReadOverlappedStruct));
}

Socket::Socket(SOCKET sck)
{
	int result = WSAStartup(MAKEWORD(2, 2), &mWData);
	if (result != 0) throw Exception("Failed to initialize WSA");
	mHandle = sck;
	memset(mReceiveBuffer, 0, sizeof(mReceiveBuffer));
	ZeroMemory(&mReadOverlappedStruct, sizeof(mReadOverlappedStruct));
}

Socket::~Socket()
{
	Close();
}

void Socket::Close()
{
	// ���� �ڵ��� �����Ѵ�.
	closesocket(mHandle);

	// ���� ���α׷��� ���� ��Ÿ���� �Լ��� �ҷ��ش�.
	WSACleanup();
}

void Socket::Bind(const EndPoint& ep)
{
	int result = bind(mHandle, (sockaddr*)&ep.mIPv4Endpoint, sizeof(ep.mIPv4Endpoint));
	if (result == SOCKET_ERROR) {
		std::stringstream ss;
		ss << "Failed to bind : " << GetLastErrorMessage();
		throw Exception(ss.str().c_str());
	}
}

int Socket::Connect(const EndPoint& ep)
{
	return connect(mHandle, (sockaddr*)&ep.mIPv4Endpoint, sizeof(ep.mIPv4Endpoint));
}

void Socket::Listen()
{
	// backlog : ���ÿ� ������ �õ��ϴ� �ִ� Ŭ���̾�Ʈ�� ��
	listen(mHandle, 5000);
}

int Socket::Accept(Socket& acceptSocket, std::string& errorString)
{
	// accept�� �����û�� �� ������ ����ϴٰ�
	// ���� ť���� �ϳ��� ������ ���ο� ������ �����Ѵ�.
	acceptSocket.mHandle = accept(mHandle, NULL, 0);
	if (acceptSocket.mHandle == SOCKET_ERROR)
	{
		errorString = GetLastErrorMessage();
		return -1;
	}
	return 0;
}

bool Socket::AcceptOverlapped(Socket& acceptCandidateSocket, std::string& errorString)
{
	if (mAcceptEx == NULL)
	{
		DWORD bytes;
		// ���� AcceptEx �Լ� �����͸� �����´�.
		WSAIoctl(mHandle,
			SIO_GET_EXTENSION_FUNCTION_POINTER,
			&UUID(WSAID_ACCEPTEX),
			sizeof(UUID),
			&mAcceptEx,
			sizeof(mAcceptEx),
			&bytes,
			NULL, NULL);
		if (mAcceptEx == NULL)
			throw Exception("Getting AcceptEx ptr failed");
	}

	// Accept�� ������ �����ּҿ� ����Ʈ �ּҰ� ä������.
	char ignored[200]{};
	DWORD ignored2 = 0;

	// ���ۿ� ������ ���� �ּҿ� ����Ʈ �ּҸ� �ִ´�.
	// acceptCandidate ������ ������ �޴´�.
	BOOL ret = mAcceptEx(mHandle, acceptCandidateSocket.mHandle,
		&ignored, 0, 50, 50, &ignored2, &mReadOverlappedStruct);

	return (ret == TRUE) ? true : false;
}

// AcceptEx�� I/O �ϷḦ �ϴ��� ���� TCP ���� ó���� ������ �ʾұ� ������
// �� �Լ��� ȣ�����־�� ���������� �Ϸ�ȴ�.
int Socket::UpdateAcceptContext(Socket& listenSocket)
{
	sockaddr_in ignore1;
	sockaddr_in ignore3;
	INT ignore2, ignore4;
	
	char ignore[1000]{};
	// AcceptEx �Լ� ȣ�� �� ���� �����͸� ���ð� ����Ʈ �ּҷ� �����ش�.
	// ������ ù��° ���� ���ڴ� AcceptEx���� ����ߴ� ���ۿ� ���� �����̴�.
	GetAcceptExSockaddrs(ignore, 0, 50, 50,
		(sockaddr**)&ignore1, &ignore2, (sockaddr**)&ignore3, &ignore4);

	// �ɼ��� ������ ����(SOL_SOCKET)�� �������ְ� 
	// ���� �����Ϳ� ����� ��ü���� �ɼ��� �����Ѵ�.
	// �� ��� ������ ���Ͽ� ����ִ� accept �ɼ��� �� ���Ͽ� �����Ѵ�.
	return setsockopt(mHandle, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
		(char*)&listenSocket.mHandle, sizeof(listenSocket.mHandle));
}

int Socket::Send(const std::string& data)
{
	return send(mHandle, data.c_str(), data.length(), 0);
}

int Socket::Send(const char* data, int length)
{
	return send(mHandle, data, length, 0);
}

int Socket::Receive()
{
	// ���Ź��ۿ� ���޹��� �����͸� �����Ѵ�.
	return recv(mHandle, mReceiveBuffer, sizeof(mReceiveBuffer), 0);
}

// Overlapped ������ �Ѵ�.
// �񵿱������� ���� ���ۿ� �����Ͱ� ä������.
int Socket::ReceiveOverlapped()
{
	WSABUF b;
	b.buf = mReceiveBuffer;
	b.len = MaxReceiveBufferLength;

	mReadFlags = 0;

	// NumberOfBytesRecvd�� overlapped structure�� NULL�� �ƴ� ������ NULL�� �� �� �ִ�.
	return WSARecv(mHandle, &b, 1, NULL, &mReadFlags, &mReadOverlappedStruct, NULL);
}

EndPoint Socket::GetPeerAddr()
{
	EndPoint ret;
	socklen_t retLength = sizeof(ret.mIPv4Endpoint);
	if (getpeername(mHandle, (sockaddr*)&ret.mIPv4Endpoint, &retLength) < 0)
	{
		std::stringstream ss;
		ss << "GetPeerAddr Failed : " << GetLastErrorMessage();
		throw Exception(ss.str().c_str());
	}
	
	// ���� ������ ���̸� �Ѿ�� ���� �߻�
	if (retLength > sizeof(ret.mIPv4Endpoint))
	{
		std::stringstream ss;
		ss << "GetPeerAddr buffer overrun : " << retLength;
		throw Exception(ss.str().c_str());
	}
	return ret;
}

void Socket::SetNonBlocking()
{
	u_long val = 1;
	int ret = ioctlsocket(mHandle, FIONBIO, &val);
	if (ret == SOCKET_ERROR)
	{
		std::stringstream ss;
		ss << "Bind Failed : " << GetLastErrorMessage();
		throw Exception(ss.str().c_str());
	}
}

