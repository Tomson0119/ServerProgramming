#include "stdafx.h"
#include "socket.h"
#include "exception.h"
#include "endpoint.h"

Socket::Socket()
	: mSck{}
{
	InitWSA();
	memset(mReceiveBuffer, 0, sizeof(mReceiveBuffer));
}

Socket::Socket(SocketType type)
{
	InitWSA();
	memset(mReceiveBuffer, 0, sizeof(mReceiveBuffer));

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
	memset(mReceiveBuffer, 0, sizeof(mReceiveBuffer));

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
	// backlog: �ִ�� ���� �� �ִ� ���� ť�� ũ��
	// -> SOMAXCONN: ������ backlog�� �������ش�. 
	if (listen(mSck, 5000) == -1)	// SOCKET_ERROR
	{
		std::stringstream ss;
		ss << "Listening failed: " << GetLastErrorLog();
		throw Exception(ss.str().c_str());
	}
}

Socket Socket::Accept()
{
	// param(2): ����� �ּҸ� ������ �� �ִ�.
	// param(3): param(2)�� ũ��
	SOCKET acpt = accept(mSck, NULL, 0);
	if (acpt == -1)	// INVALID_SOCKET
	{
		std::stringstream ss;
		ss << "Accept failed: " << GetLastErrorLog();
		throw Exception(ss.str().c_str());
	}
	return Socket(acpt);
}

int Socket::Send(const char* data, int length)
{
	// param(4): MSG_DONTROUT(������� ���� �ʴ´�.)
	//			 MSG_OOB(Out of band ��Ʈ�� �����͸� ������.)
	int len = send(mSck, data, length, 0);
	if (len == -1)
	{
		if (!mNonBlocking) // ���ŷ ������ ���
		{
			std::stringstream ss;
			ss << "Send failed: " << GetLastErrorLog();
			throw Exception(ss.str().c_str());
		}
		else			  // ����ŷ ������ ���
		{
		#ifdef _WIN32
			return WSAGetLastError();
		#else
			return errno;
		#endif
		}
	}
}

int Socket::Receive()
{
	// param(4): MSG_PEEK(�ڿ� ���� �����͸� �̸� ���⸸ �Ѵ�.)
	//			 MSG_OOB(Out of band ������ ó��)
	//			 MSG_WAITALL(Ư���� ��Ȳ������ ���Ź޴´�.)
	int len = recv(mSck, mReceiveBuffer, MaxReceiveLength, 0);
	if (len == -1)	// SOCKET_ERROR
	{
		if (!mNonBlocking)	// ���ŷ ������ ���
		{
			std::stringstream ss;
			ss << "Receive failed: " << GetLastErrorLog();
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
	return len;
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

EndPoint Socket::GetPeerAddr()
{
	EndPoint ret;
	socklen_t retLength = sizeof(ret.mAddr);

	// ����� ������ �ּҸ� ��ȯ�Ѵ�.
	if (getpeername(mSck, reinterpret_cast<sockaddr*>(&ret.mAddr), 
		&retLength)	== -1)	// SOCKET_ERROR
	{
		std::stringstream ss;
		ss << "GetPeerAddr failed: " << GetLastErrorLog();
		throw Exception(ss.str().c_str());
	}

	// �ּ� ������ ���̸� �Ѿ�� �ȵȴ�.
	if (retLength > sizeof(ret.mAddr))
	{
		std::stringstream ss;
		ss << "GetPeerAddr buffer overrun: " << retLength;
		throw Exception(ss.str().c_str());
	}
	return ret;
}



