#pragma once

#include "stdafx.h"
#include "Socket.h"
#include "Protocol.h"
#include "BufferQueue.h"
#include "MemoryPoolManager.h"

enum class OP : char
{
	RECV,
	SEND,
	ACCEPT,
	NPC_MOVE,
	PLAYER_MOVE
};

struct WSAOVERLAPPEDEX
{
	WSAOVERLAPPED Overlapped;
	WSABUF WSABuffer;
	OP Operation;
	BufferQueue NetBuffer;

	int Target;

	WSAOVERLAPPEDEX(OP op = OP::RECV)
		: Operation(op), WSABuffer{}
	{
		Reset(op);
	}

	WSAOVERLAPPEDEX(OP op, std::byte* data, int bytes)
		: Operation(op), WSABuffer{}
	{
		Reset(op, data, bytes);
	}

	void Reset(OP op, std::byte* data = nullptr, int bytes = 0)
	{
		Operation = op;
		ZeroMemory(&Overlapped, sizeof(Overlapped));

		if (bytes > 0 && data)
		{
			NetBuffer.Clear();
			NetBuffer.Push(data, bytes);
			WSABuffer.buf = reinterpret_cast<char*>(NetBuffer.BufStartPtr());
			WSABuffer.len = (ULONG)bytes;
		}
		else
		{
			WSABuffer.buf = reinterpret_cast<char*>(NetBuffer.BufWritePtr());
			WSABuffer.len = (ULONG)NetBuffer.GetLeftBufLen();
		}
	}

	void PushMsg(std::byte* data, int bytes)
	{
		if (Operation == OP::SEND)
		{
			// Multi thread에서 Push를 수행할 수 있으므로 lock.
			std::unique_lock<std::mutex> lock{ PushMut };

			NetBuffer.Push(data, bytes);
			WSABuffer.len = (ULONG)NetBuffer.GetFilledBufLen();
		}
	}

	void* operator new(size_t size)
	{
		if (MemoryPoolManager<WSAOVERLAPPEDEX>::GetInstance().GetPoolSize() > 0)
		{
			auto p = MemoryPoolManager<WSAOVERLAPPEDEX>::GetInstance().Allocate();
			if (p) return p;
		}
		return ::operator new(size);
	}

	void operator delete(void* ptr)
	{
		if (MemoryPoolManager<WSAOVERLAPPEDEX>::GetInstance().GetPoolSize() > 0)
			MemoryPoolManager<WSAOVERLAPPEDEX>::GetInstance().Deallocate(ptr);
		else
			::operator delete(ptr);
	}

private:
	std::mutex PushMut;
};