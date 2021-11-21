#pragma once

#include "stdafx.h"
#include "Socket.h"
#include "Protocol.h"

enum class OP : char
{
	RECV,
	SEND,
	ACCEPT
};

struct WSAOVERLAPPEDEX
{
	WSAOVERLAPPED Overlapped;
	WSABUF WSABuffer;
	OP Operation;
	uchar NetBuffer[MaxBufferSize];

	WSAOVERLAPPEDEX(OP op = OP::RECV)
		: Operation(op), WSABuffer{}, NetBuffer{}
	{
		ZeroMemory(&Overlapped, sizeof(Overlapped));
		WSABuffer.buf = reinterpret_cast<char*>(NetBuffer);
		WSABuffer.len = sizeof(NetBuffer);
	}

	WSAOVERLAPPEDEX(OP op, char* data, int bytes)
		: Operation(op), WSABuffer{}, NetBuffer{}
	{
		Reset(op, data, bytes);
	}

	void Reset(OP op)
	{
		Operation = op;
		ZeroMemory(&Overlapped, sizeof(Overlapped));
		WSABuffer.buf = reinterpret_cast<char*>(NetBuffer);
		WSABuffer.len = MaxBufferSize;
	}

	void Reset(OP op, char* data, int bytes)
	{
		Operation = op;
		ZeroMemory(&Overlapped, sizeof(Overlapped));
		std::memcpy(NetBuffer, data, bytes);
		WSABuffer.buf = reinterpret_cast<char*>(NetBuffer);
		WSABuffer.len = (ULONG)bytes;
	}
};