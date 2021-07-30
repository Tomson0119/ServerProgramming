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
	// backlog: 최대로 넣을 수 있는 연결 큐의 크기
	// -> SOMAXCONN: 적절한 backlog로 설정해준다. 
	if (listen(mSck, 5000) == -1)	// SOCKET_ERROR
	{
		std::stringstream ss;
		ss << "Listening failed: " << GetLastErrorLog();
		throw Exception(ss.str().c_str());
	}
}

Socket Socket::Accept()
{
	// param(2): 연결된 주소를 복사할 수 있다.
	// param(3): param(2)의 크기
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
	// param(4): MSG_DONTROUT(라우팅을 하지 않는다.)
	//			 MSG_OOB(Out of band 스트림 데이터를 보낸다.)
	int len = send(mSck, data, length, 0);
	if (len == -1)
	{
		if (!mNonBlocking) // 블록킹 소켓일 경우
		{
			std::stringstream ss;
			ss << "Send failed: " << GetLastErrorLog();
			throw Exception(ss.str().c_str());
		}
		else			  // 논블록킹 소켓일 경우
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
	// param(4): MSG_PEEK(뒤에 오는 데이터를 미리 보기만 한다.)
	//			 MSG_OOB(Out of band 데이터 처리)
	//			 MSG_WAITALL(특별한 상황에서만 수신받는다.)
	int len = recv(mSck, mReceiveBuffer, MaxReceiveLength, 0);
	if (len == -1)	// SOCKET_ERROR
	{
		if (!mNonBlocking)	// 블록킹 소켓일 경우
		{
			std::stringstream ss;
			ss << "Receive failed: " << GetLastErrorLog();
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
	return len;
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

EndPoint Socket::GetPeerAddr()
{
	EndPoint ret;
	socklen_t retLength = sizeof(ret.mAddr);

	// 연결된 소켓의 주소를 반환한다.
	if (getpeername(mSck, reinterpret_cast<sockaddr*>(&ret.mAddr), 
		&retLength)	== -1)	// SOCKET_ERROR
	{
		std::stringstream ss;
		ss << "GetPeerAddr failed: " << GetLastErrorLog();
		throw Exception(ss.str().c_str());
	}

	// 주소 버퍼의 길이를 넘어서면 안된다.
	if (retLength > sizeof(ret.mAddr))
	{
		std::stringstream ss;
		ss << "GetPeerAddr buffer overrun: " << retLength;
		throw Exception(ss.str().c_str());
	}
	return ret;
}



