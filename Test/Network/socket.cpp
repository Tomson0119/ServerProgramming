#include "stdafx.h"
#include "socket.h"
#include "exception.h"
#include "endpoint.h"

Socket::Socket()
	: mSck{}
{
	InitWSA();
}

Socket::Socket(SocketType type)
{
	InitWSA();

	if (type == SocketType::TCP)
	{
		mSck = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}
	else if (type == SocketType::UDP)
	{
		mSck = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}

#ifdef _WIN32
	if (type == SocketType::OVERLAPPED_TCP)
	{
		mSck = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	}
	else if (type == SocketType::OVERLAPPED_UDP)
	{
		mSck = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	}
	ZeroMemory(&mReadOverlappedStruct, sizeof(mReadOverlappedStruct));
#endif
}

Socket::Socket(SOCKET sck)
{
	InitWSA();
	mSck = sck;

#ifdef _WIN32
	ZeroMemory(&mReadOverlappedStruct, sizeof(mReadOverlappedStruct));
#endif
}

Socket::~Socket()
{
	Close();
}

void Socket::InitWSA()
{
#ifdef _WIN32
	// WSA�� �����Ѵ�. ������ �߻��ϸ� ����ó���Ѵ�.
	// �����ڵ带 ���ڴٰ� WSAGetLastError() �Լ��� ����ؼ��� �ȵȴ�.
	if(WSAStartup(MAKEWORD(2, 3), &mWData) != 0)
	{
		throw Exception("Failed to start WSA");
	}
#else
#endif
}

void Socket::Close()
{
	// ������ �ݰ�, WSA�� ������.
#ifdef _WIN32
	closesocket(mSck);
	WSACleanup();
#else
	close(mSck);
#endif	
}

void Socket::Bind(const EndPoint& ep)
{
	// ���ε��� �����ϸ� 0�� ��ȯ�Ѵ�.
	// �����ϸ� SOCKET_ERROR�� ��ȯ�ϸ�, WSAGetLastError() �Լ���
	// ��ü���� �����ڵ带 Ȯ���� �� �ִ�.
	if(bind(mSck, reinterpret_cast<const sockaddr*>(&ep.mAddr), 
		sizeof(ep.mAddr)) == -1)	// SOCKET_ERROR
	{
		std::stringstream ss;
		ss << "Bind failed: " << GetLastErrorLog();
		throw Exception(ss.str().c_str());
	}
}

int Socket::Connect(const EndPoint& ep)
{
	// ������ �ּҷ� ���� ��û�� ������.
	// �����ϸ� SOCKET_ERROR�� ��ȯ�ϰ�, WSAGetLastError() �Լ���
	// ��ü���� �����ڵ带 Ȯ���� �� �ִ�.
	if (connect(mSck, reinterpret_cast<const sockaddr*>(&ep.mAddr),
		sizeof(ep.mAddr)) == -1)	// SOCKET_ERROR
	{
		if (!mNonBlocking)	// ���ŷ ������ ���
		{
			std::stringstream ss;
			ss << "Connection failed: " << GetLastErrorLog();
			throw Exception(ss.str().c_str());
		}
		else				// ����ŷ ������ ���
		{
		#ifdef _WIN32
			return WSAGetLastError();
		#else
			return errno;
		#endif
		}
	}
	return 0;
}

void Socket::Listen()
{
}

Socket Socket::Accept(std::string& errorText)
{
	return Socket();
}

void Socket::SetNonBlocking()
{
	u_long val = 1;

#ifdef _WIN32
	// FIONBIO�� Blocking ��带 �ǹ��ϸ�
	// 0���� �����ϸ� ���ŷ ��尡 �ǰ�,
	// 1�� �����ϸ� ����ŷ ��尡 �ȴ�.
	int result = ioctlsocket(mSck, FIONBIO, &val);
#else
	int result = ioctl(mSck, FIONBIO, &val);
#endif

	if (result == -1)	// SOCKET_ERROR
	{
		std::stringstream ss;
		ss << "Setting non-blocking failed: " << GetLastErrorLog();
		throw Exception(ss.str().c_str());
	}
	else
	{
		// ���� ���¸� ��Ÿ���� �÷��׸� �����Ѵ�.
		mNonBlocking = true;
	}
}



