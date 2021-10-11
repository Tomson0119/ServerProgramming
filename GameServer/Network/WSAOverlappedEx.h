#pragma once

#include "stdafx.h"
#include "Socket.h"
#include "Message.h"

struct WSAOVERLAPPEDEX : WSAOVERLAPPED
{
	WSABUF WSABuffer;
	class Socket* Caller;

	WSAOVERLAPPEDEX()
		: WSABuffer{}
	{
	}

	WSAOVERLAPPEDEX(int bytes, char* data, Socket* caller)
		: WSABuffer{}
	{ 
		WSABuffer.buf = data;
		WSABuffer.len = (ULONG)bytes;
		Caller = caller;
	}
};