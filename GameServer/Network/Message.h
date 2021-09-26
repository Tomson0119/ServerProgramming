#pragma once

#include <vector>

enum class MsgType
{
	MSG_MOVE
};

class Message
{
public:
	ushort_t size = 0;
	MsgType mMsgType{};

public:
	Message() { }
	Message(MsgType type) : mMsgType(type) { }
	~Message() { }

	template<typename T>
	void Push(const T& data)
	{
		size_t dataSize = sizeof(T);
		size_t currSize = mPackets.size();

		mPackets.resize(currSize + dataSize);
		std::memcpy(mPackets.data() + currSize, &data, dataSize);

		size = mPackets.size();
	}

	template<typename T>
	void Pop(T& data)
	{
		size_t dataSize = sizeof(T);
		size_t start = mPackets.size() - dataSize;
		std::memcpy(&data, mPackets.data() + start, dataSize);
		mPackets.resize(start);
	}

	std::vector<uint8_t> GetSendPackets()
	{
		mPackets.insert(mPackets.begin(), (uint8_t)mMsgType);
		mPackets.insert(mPackets.begin(), LOBYTE(size));
		mPackets.insert(mPackets.begin(), HIBYTE(size));
	
		return mPackets;
	}

public:
	std::vector<uint8_t> mPackets;
};
