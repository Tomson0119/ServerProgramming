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
	// WSA를 시작한다. 에러가 발생하면 예외처리한다.
	// 에러코드를 보겠다고 WSAGetLastError() 함수를 사용해서는 안된다.
	if(WSAStartup(MAKEWORD(2, 3), &mWData) != 0)
	{
		throw Exception("Failed to start WSA");
	}
#else
#endif
}

void Socket::Close()
{
	// 소켓을 닫고, WSA를 끝낸다.
#ifdef _WIN32
	closesocket(mSck);
	WSACleanup();
#else
	close(mSck);
#endif	
}

void Socket::Bind(const EndPoint& ep)
{
	// 바인딩에 성공하면 0을 반환한다.
	// 실패하면 SOCKET_ERROR를 반환하며, WSAGetLastError() 함수로
	// 구체적인 에러코드를 확인할 수 있다.
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
	// 지정한 주소로 연결 요청을 보낸다.
	// 실패하면 SOCKET_ERROR를 반환하고, WSAGetLastError() 함수로
	// 구체적인 에러코드를 확인할 수 있다.
	if (connect(mSck, reinterpret_cast<const sockaddr*>(&ep.mAddr),
		sizeof(ep.mAddr)) == -1)	// SOCKET_ERROR
	{
		if (!mNonBlocking)	// 블록킹 소켓일 경우
		{
			std::stringstream ss;
			ss << "Connection failed: " << GetLastErrorLog();
			throw Exception(ss.str().c_str());
		}
		else				// 논블록킹 소켓일 경우
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
	// FIONBIO는 Blocking 모드를 의미하며
	// 0으로 설정하면 블록킹 모드가 되고,
	// 1로 설정하면 논블록킹 모드가 된다.
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
		// 논블록 상태를 나타내는 플래그를 설정한다.
		mNonBlocking = true;
	}
}



