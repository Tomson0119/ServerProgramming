#pragma once

#include "Protocol.h"

class RingBuffer
{
public:
	RingBuffer();
	~RingBuffer();

	void Clear();

	void Push(uchar* msg, int size);
	void Pop(uchar* msg, int size);

	bool IsEmpty();
	bool IsFull();

	char GetMsgType();
	uchar GetTotalMsgSize();
	uchar PeekNextPacketSize();

	uchar m_buffer[MaxBufferSize];

private:
	int m_readIndex;
	int m_writeIndex;
	int m_remainSize;
};