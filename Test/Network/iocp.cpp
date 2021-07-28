#include "stdafx.h"
#include "iocp.h"
#include "socket.h"
#include "exception.h"

IOCP::IOCP(int threadCount)
{
	// ����� �Ϸ� ��Ʈ�� �����Ѵ�.
	// ù��° ����: ���� �ڵ�, ������ invalid
	// �ι�° ����: �̹� �����ϴ� ����� �Ϸ� ��Ʈ, ������ null
	// ����° ����: �ڵ��� ����Ű�� Ű�̴�.
	// �׹�° ����: �Ҵ��� �������� �����̴�.
	// �� ��� IOCP �ڵ��� �����ϱ� ���� ����ߴ�.
	mHIocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, threadCount);
	mThreadCount = threadCount;
}

IOCP::~IOCP()
{
	CloseHandle(mHIocp);
}

void IOCP::Add(Socket& sck, void* userPtr)
{
	// ���ڷ� �Ѿ�� ������ �ڵ��� IOCP �ڵ鿡 �����Ѵ�.
	// ������ IOCP�� �߰��ߴٰ� ǥ���� �� �ִ�.
	if (!CreateIoCompletionPort((HANDLE)sck.mHandle, mHIocp, 
		(ULONG_PTR)userPtr, mThreadCount))
		throw Exception("Failed to add IOCP");
}

void IOCP::Wait(IocpEvents& output, int timeOut)
{
	// ť���� �̺�Ʈ�� ������. timeOut���� ��ٸ���.
	// I/O �� �߻��� ����Ʈ��, �Ϸ��ȣ, ������ ����ü �ּҰ���
	// ��� entry�� ��ȯ�ȴ�.
	// entryRemoved�� ������ ������ entry�� ������ ��´�.
	// alterable�� FALSE�̸� entry�� �ްų� �ð��� ����� ������ ��ٸ���.
	BOOL result = GetQueuedCompletionStatusEx(mHIocp, output.mEvents,
		MaxEvents, (ULONG*)&output.mEventCount, timeOut, FALSE);

	// �����Ѵٸ� ��ȯ�� �̺�Ʈ�� ������ �ٽ� 0���� �ʱ�ȭ�Ѵ�.
	if (!result)
		output.mEventCount = 0;
}
