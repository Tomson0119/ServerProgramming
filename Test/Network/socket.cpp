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
	// 윈도우 소켓프로그래밍에서 제일 먼저 호출하는 함수이다.
	int result = WSAStartup(MAKEWORD(2, 2), &mWData); // 2.2 버전
	if (result != 0) throw Exception("Failed to initialize WSA");

	if (type == SocketType::TCP)
	{
		if (overlapped)
		{
			// 오버랩 TCP 소켓을 만든다.
			mHandle = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		}
		else
		{
			// 소켓 핸들을 생성한다. (AF_INET : IPv4, AF_INET : IPv6)
			// 스트림 데이터를 다루는 TCP 프로토콜을 생성한다.
			mHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		}
		
	}
	else
	{
		if (overlapped)
		{
			// 오버랩 UDP 소켓을 만든다.
			mHandle = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
		}
		else
		{
			// 데이터그램을 다루는 UDP 프로토콜을 생성한다.
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
	// 소켓 핸들을 종료한다.
	closesocket(mHandle);

	// 소켓 프로그램의 끝을 나타내는 함수를 불러준다.
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
	// backlog : 동시에 연결을 시도하는 최대 클라이언트의 수
	listen(mHandle, 5000);
}

int Socket::Accept(Socket& acceptSocket, std::string& errorString)
{
	// accept는 연결요청이 올 때까지 대기하다가
	// 연결 큐에서 하나를 꺼내어 새로운 소켓을 생성한다.
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
		// 먼저 AcceptEx 함수 포인터를 가져온다.
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

	// Accept된 소켓의 로컬주소와 리모트 주소가 채워진다.
	char ignored[200]{};
	DWORD ignored2 = 0;

	// 버퍼에 소켓의 로컬 주소와 리모트 주소를 넣는다.
	// acceptCandidate 소켓은 연결을 받는다.
	BOOL ret = mAcceptEx(mHandle, acceptCandidateSocket.mHandle,
		&ignored, 0, 50, 50, &ignored2, &mReadOverlappedStruct);

	return (ret == TRUE) ? true : false;
}

// AcceptEx가 I/O 완료를 하더라도 아직 TCP 연결 처리가 끝나지 않았기 때문에
// 이 함수를 호출해주어야 최종적으로 완료된다.
int Socket::UpdateAcceptContext(Socket& listenSocket)
{
	sockaddr_in ignore1;
	sockaddr_in ignore3;
	INT ignore2, ignore4;
	
	char ignore[1000]{};
	// AcceptEx 함수 호출 후 받은 데이터를 로컬과 리모트 주소로 나눠준다.
	// 보통은 첫번째 버퍼 인자는 AcceptEx에서 사용했던 버퍼와 같은 변수이다.
	GetAcceptExSockaddrs(ignore, 0, 50, 50,
		(sockaddr**)&ignore1, &ignore2, (sockaddr**)&ignore3, &ignore4);

	// 옵션이 지정된 레벨(SOL_SOCKET)을 지정해주고 
	// 버퍼 포인터에 담겨진 구체적인 옵션을 설정한다.
	// 이 경우 리스닝 소켓에 담겨있는 accept 옵션을 이 소켓에 설정한다.
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
	// 수신버퍼에 전달받은 데이터를 저장한다.
	return recv(mHandle, mReceiveBuffer, sizeof(mReceiveBuffer), 0);
}

// Overlapped 수신을 한다.
// 비동기적으로 수신 버퍼에 데이터가 채워진다.
int Socket::ReceiveOverlapped()
{
	WSABUF b;
	b.buf = mReceiveBuffer;
	b.len = MaxReceiveBufferLength;

	mReadFlags = 0;

	// NumberOfBytesRecvd는 overlapped structure이 NULL이 아닐 때에만 NULL이 될 수 있다.
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
	
	// 끝점 버퍼의 길이를 넘어서면 예외 발생
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

