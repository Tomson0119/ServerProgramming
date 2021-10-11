#pragma once

#include <vector>

enum class MsgType : uint8_t
{
	MSG_ACCEPT,
	MSG_JOIN,
	MSG_MOVE,
	MSG_DISCONNECT
};

struct MsgHeader
{
	uint8_t size;
	MsgType msg_type;
};

const int MaxRecvSize = 1024;

class Message
{
public:
	MsgHeader Header{};

public:
	Message() { }
	Message(MsgType type) 
	{
		Header.size = 0;
		Header.msg_type = type;
	}
	~Message() { }

	template<typename T>
	void Push(const T& data)
	{
		size_t dataSize = sizeof(T);
		size_t currSize = Header.size;

		std::memcpy(Body.data() + currSize, &data, dataSize);

		Header.size += (uint8_t)dataSize;
	}

	size_t GetPacketSize() const
	{
		return (sizeof(MsgHeader) + Header.size);
	}

public:
	std::array<uint8_t, MaxRecvSize-sizeof(MsgHeader)> Body;
};
