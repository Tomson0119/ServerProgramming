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

	// 소켓을 삭제한다.
	void Close();

	// 소켓을 Endpoint에 할당한다.
	void Bind(const EndPoint& ep);

	// 서버 Endpoint에 연결을 요청한다.
	int Connect(const EndPoint& ep);

	// 연결 요청이 올 때까지 대기한다.
	void Listen();

	// 클라이언트와의 연결을 허용하고 새로운 소켓을 생성한다.
	int Accept(Socket& acceptSocket, std::string& errorString);

	// Overlapped I/O에서 소켓 연결을 담당한다.
	bool AcceptOverlapped(Socket& acceptCandidateSocket, std::string& errorString);
	int UpdateAcceptContext(Socket& listenSocket);

	int Send(const std::string& data);
	int Send(const char* data, int length);

	int Receive();
	int ReceiveOverlapped();

	// 연결한 상태에서 상대방 끝점을 반환한다. 
	EndPoint GetPeerAddr();

	// 논블록 소켓으로 설정한다.
	void SetNonBlocking();

	static const int MaxReceiveBufferLength = 256;
	char mReceiveBuffer[MaxReceiveBufferLength];

private:
	// 소켓 구현을 위한 초기화에 쓰이는 구조체
	WSADATA mWData;
	
	// AcceptEx 함수 포인터
	LPFN_ACCEPTEX mAcceptEx = NULL;

public:
	// Overlapped I/O, IOCP를 쓸 때에만 사용한다.
	// 현재 overlapped I/O 중이면 true이다.
	// N-send, N-recv도 가능하게 하려면 이렇게 단일 값만 저장해서는 안 된다.
	bool mIsReadOverlapped = false;

private:
	// Overlapped receive or accept를 할 때 사용되는 overlapped 객체.
	// I/O 완료 전까지는 보존되어야 한다.
	WSAOVERLAPPED mReadOverlappedStruct;

	// Overlapped 수신을 하는 동안 recv의 flags에 준하는 값이다.
	// I/O가 완료될 때까지 건드려선 안 된다.
	DWORD mReadFlags = 0;

public:
	SOCKET mHandle;
};

