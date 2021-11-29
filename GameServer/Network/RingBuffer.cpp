#include "stdafx.h"
#include "RingBuffer.h"

RingBuffer::RingBuffer()
	: m_writeIndex(0),
	m_readIndex(0),
	m_remainSize(MaxBufferSize)
{
	std::memset(m_buffer, 0, MaxBufferSize);
}

RingBuffer::~RingBuffer()
{
}

void RingBuffer::Clear()
{
	std::memset(m_buffer, 0, MaxBufferSize);
	m_writeIndex = 0;
	m_readIndex = 0;
	m_remainSize = MaxBufferSize;
}

void RingBuffer::Push(uchar* msg, int size)
{
	if (size <= 0)
		return;
	if (IsFull())
	{
		std::cout << "Buffer overflow!\n";
		return;
	}

	int push_amount = 0;
	if (m_remainSize >= size)
	{
		bool out_of_index = (m_writeIndex + size) > MaxBufferSize;
		if (out_of_index)
			push_amount = MaxBufferSize - m_writeIndex;
		else
			push_amount = size;
	}
	else
		push_amount = m_remainSize;

	std::memcpy(m_buffer + m_writeIndex, msg, push_amount);
	m_remainSize -= push_amount;
	m_writeIndex = (m_writeIndex + push_amount) % MaxBufferSize;

	Push(msg + push_amount, size - push_amount);
}

void RingBuffer::Pop(uchar* msg, int size)
{
	if (IsEmpty() || size <= 0)
		return;

	int pop_amount = 0;
	int remain_data_size = MaxBufferSize - m_remainSize;
	if (remain_data_size >= size)
	{
		bool out_of_index = (m_readIndex + size) > MaxBufferSize;
		if (out_of_index)
			pop_amount = MaxBufferSize - m_readIndex;
		else
			pop_amount = size;
	}
	else
	{
		pop_amount = remain_data_size;
	}

	std::memcpy(msg, m_buffer + m_readIndex, pop_amount);
	m_remainSize += pop_amount;
	m_readIndex = (m_readIndex + pop_amount) % MaxBufferSize;

	Pop(msg + pop_amount, size - pop_amount);
}

bool RingBuffer::IsEmpty()
{
	return m_writeIndex == m_readIndex;
}

bool RingBuffer::IsFull()
{
	return m_remainSize == 0;
}

char RingBuffer::GetMsgType()
{
	char type;
	int typeIndex = (m_readIndex + 1) % MaxBufferSize;
	std::memcpy(reinterpret_cast<void*>(&type), m_buffer + typeIndex, sizeof(char));
	return type;
}

uchar RingBuffer::GetTotalMsgSize()
{
	return (uchar)(MaxBufferSize - m_remainSize);
}

uchar RingBuffer::PeekNextPacketSize()
{
	uchar size = 0;
	std::memcpy(reinterpret_cast<void*>(&size), m_buffer + m_readIndex, sizeof(size));
	return size;
}
