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

	// �ѹ��� GetQueuedCompletionStatus�� ���� �� �ִ� �ִ� ���� ��
	static const int MaxEvents = 1000;

	HANDLE mHIocp;
	int mThreadCount;
};

struct IocpEvents
{
	// I/O ť���� ������ �̺�Ʈ���� ��� �ִ�.
	OVERLAPPED_ENTRY mEvents[IOCP::MaxEvents];
	int mEventCount;
};