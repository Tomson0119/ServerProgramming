#include "stdafx.h"
#include "BufferQueue.h"


BufferQueue::BufferQueue()
	: mWriteIndex(0),
	mReadIndex(0),
	mRemainSize(MaxBufferSize)
{
	std::memset(mBuffer, 0, MaxBufferSize);
}

BufferQueue::~BufferQueue()
{
}

void BufferQueue::Clear()
{
	mWriteIndex = 0;
	mReadIndex = 0;
	mRemainSize = MaxBufferSize;
}

void BufferQueue::Push(std::byte* msg, int size)
{
	if (msg != nullptr && size > 0)
	{
		if (mRemainSize < size) {
			std::cout << "[Buffer Overflow] Unable to push data.\n";
			return;
		}
		std::memcpy(mBuffer + mWriteIndex, msg, size);
		mWriteIndex += size;
		mRemainSize -= size;
	}
}

void BufferQueue::ShiftWritePtr(int offset)
{
	if (GetLeftBufLen() < offset) {
		std::cout << "[Buffer Overflow] Unable to shift.\n";
		return;
	}

	if (GetLeftBufLen() == offset) {
		std::memcpy(mBuffer, mBuffer + mReadIndex, GetFilledBufLen() + offset);
		mReadIndex = 0;
		mWriteIndex = GetFilledBufLen();
	}
	mWriteIndex += offset;
	mRemainSize -= offset;
}

bool BufferQueue::Readable()
{
	if (GetFilledBufLen() >= sizeof(char))
		return (Empty() == false && PeekNextPacketSize() <= GetFilledBufLen());
	else
		return false;
}

char BufferQueue::PeekNextPacketSize()
{
	return *reinterpret_cast<char*>(mBuffer + mReadIndex);
}

std::byte* BufferQueue::BufReadPtr()
{
	if (Empty()) {
		std::cout << "[Buffer Is Empty]\n";
		return nullptr;
	}

	int size = (int)PeekNextPacketSize();
	if (size <= GetFilledBufLen())
	{
		std::byte* ptr = reinterpret_cast<std::byte*>(mBuffer + mReadIndex);
		mReadIndex += size;
		mRemainSize += size;

		return ptr;
	}
	return nullptr;
}