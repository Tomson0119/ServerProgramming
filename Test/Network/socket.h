#pragma once

#ifdef _WIN32
#include <WinSock2.h>
#include <MSWSock.h>
#else
#include <sys/socket.h>
#endif

#include <string>

#ifdef __linux__
// 리눅스에서는 소켓 핸들로 int 자료형을 사용한다.
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

	// 윈도우즈 소켓을 사용할 수 있도록 초기화한다.
	void InitWSA();

	// 소켓을 종료한다.
	void Close();

	// 소켓을 EndPoint에 할당한다.
	void Bind(const EndPoint& ep);

	// 소켓 연결을 요청한다.
	int Connect(const EndPoint& ep);

	// 연결이 들어오기를 기다린다.
	void Listen();

	// 연결을 허용하고 새로운 소켓을 생성한다.
	Socket Accept(std::string& errorText);

	// 논블록킹 소켓으로 설정한다.
	void SetNonBlocking();

private:
	// 수신 받을 수 있는 데이터의 최대 길이
	static const int MaxReceiveLength = 1024;

	SOCKET mSck;	// 소켓 핸들
	
	bool mNonBlocking = false;	// 논블록 플래그

#ifdef _WIN32
	WSADATA mWData;

	// Overlapped receive / accept를 할 때 사용하는 구조체
	// I/O 완료 전까지는 보존되어야 한다.
	WSAOVERLAPPED mReadOverlappedStruct;
	
	// Overlapped receive를 하는 동안 recv의 flags에 준하는 값.
	// I/O 완료 전까지는 보존되어야 한다.
	DWORD mReadFlags = 0;

	// AcceptEx 함수 포인터
	LPFN_ACCEPTEX mAcceptEx;
#endif
};