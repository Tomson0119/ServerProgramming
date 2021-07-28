#pragma once

class Socket;
class IocpEvents;

class IOCP
{
public:
	IOCP(int threadCount);
	~IOCP();

	void Add(Socket& sck, void* userPtr);
	void Wait(IocpEvents& output, int timeOut);

	// 한번에 GetQueuedCompletionStatus이 꺼낼 수 있는 최대 일의 수
	static const int MaxEvents = 1000;

	HANDLE mHIocp;
	int mThreadCount;
};

struct IocpEvents
{
	// I/O 큐에서 꺼내온 이벤트들을 담고 있다.
	OVERLAPPED_ENTRY mEvents[IOCP::MaxEvents];
	int mEventCount;
};