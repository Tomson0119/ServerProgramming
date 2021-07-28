#include "stdafx.h"
#include "iocp.h"
#include "socket.h"
#include "exception.h"

IOCP::IOCP(int threadCount)
{
	// 입출력 완료 포트를 생성한다.
	// 첫번째 인자: 파일 핸들, 없으면 invalid
	// 두번째 인자: 이미 존재하는 입출력 완료 포트, 없으면 null
	// 세번째 인자: 핸들을 가리키는 키이다.
	// 네번째 인자: 할당할 스레드의 개수이다.
	// 이 경우 IOCP 핸들을 생성하기 위해 사용했다.
	mHIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, threadCount);
	mThreadCount = threadCount;
}

IOCP::~IOCP()
{
	CloseHandle(mHIocp);
}

void IOCP::Add(Socket& sck, void* userPtr)
{
	// 인자로 넘어온 소켓의 핸들을 IOCP 핸들에 연동한다.
	// 소켓을 IOCP에 추가했다고 표현할 수 있다.
	if (!CreateIoCompletionPort((HANDLE)sck.mHandle, mHIocp, 
		(ULONG_PTR)userPtr, mThreadCount))
		throw Exception("Failed to add IOCP");
}

void IOCP::Wait(IocpEvents& output, int timeOut)
{
	// 큐에서 이벤트를 꺼낸다. timeOut동안 기다린다.
	// I/O 중 발생한 바이트와, 완료신호, 오버랩 구조체 주소값이
	// 모두 entry로 반환된다.
	// entryRemoved는 실제로 삭제된 entry의 개수를 담는다.
	// alterable이 FALSE이면 entry를 받거나 시간이 종료될 떄까지 기다린다.
	BOOL result = GetQueuedCompletionStatusEx(mHIocp, output.mEvents,
		MaxEvents, (ULONG*)&output.mEventCount, timeOut, FALSE);

	// 실패한다면 반환된 이벤트의 개수를 다시 0으로 초기화한다.
	if (!result)
		output.mEventCount = 0;
}
