#include "stdafx.h"
#include "IOCP.h"
#include "Socket.h"
#include "WSAOverlappedEx.h"

IOCP::IOCP()
{
	mIOCPHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
}

IOCP::~IOCP()
{
}

void IOCP::RegisterDevice(SOCKET sck, int key)
{
	if (CreateIoCompletionPort(reinterpret_cast<HANDLE>(sck), mIOCPHandle, key, 0) == NULL)
		throw NetException("Registering device failed");
}

void IOCP::PostToCompletionQueue(WSAOVERLAPPEDEX* over, int key)
{
	if (PostQueuedCompletionStatus(mIOCPHandle, 1, key, &over->Overlapped) == NULL)
		throw NetException("Posting to completion queue failed");
}

CompletionInfo IOCP::GetCompletionInfo() const
{
	CompletionInfo info{};
	info.success = GetQueuedCompletionStatus(mIOCPHandle, &info.bytes, (PULONG_PTR)&info.key, &info.overEx, INFINITE);
	return info;
}

