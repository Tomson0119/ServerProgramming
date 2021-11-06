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

void IOCP::RegisterDevice(const Socket& sck, int key)
{
	if (CreateIoCompletionPort(reinterpret_cast<HANDLE>(sck.mSocket), mIOCP, key, 0) == NULL)
		throw NetException("Registering device failed");
}

