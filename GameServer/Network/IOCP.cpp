#include "stdafx.h"
#include "IOCP.h"
#include "Socket.h"

IOCP::IOCP()
{
	mIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
}

IOCP::~IOCP()
{
}

void IOCP::RegisterDevice(SOCKET sck, int key)
{
	if (CreateIoCompletionPort(reinterpret_cast<HANDLE>(sck), mIOCP, key, 0) == NULL)
		throw NetException("Registering device failed");
}

CompletionInfo IOCP::GetCompletionInfo() const
{
	CompletionInfo info{};
	info.success = GetQueuedCompletionStatus(mIOCP, &info.bytes, (PULONG_PTR)&info.key, &info.overEx, INFINITE);
	return info;
}

