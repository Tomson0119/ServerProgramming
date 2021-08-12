#pragma once
#include "common.h"
#include <fstream>

namespace net
{
	// Message Header is sent at start of all messages.
	// The template allows us to use "enum class" to ensure
	// that the messages are valid at compile time.
	template<typename T>
	struct message_header
	{
		T id{};
		uint32_t size = 0;
	};

	template<typename T>
	struct message
	{
		message_header<T> header{};
		std::vector<uint8_t> body;	// Contains data converted to byte.
									// It functions like a stack in this case.

		// returns size of entire message packet in bytes
		size_t size() const
		{
			// Message size is header's size and all the byte size of data
			return header.size;
		}

		// Produces description of message
		friend std::ostream& operator<<(std::ostream& os, const message<T>& msg)
		{
			os << "ID: " << int(msg.header.id) << " Size: " << msg.header.size;
			return os;
		}

		void encodeString(const std::string& str, size_t length)
		{
			size_t i = body.size();

			body.resize(i + length);

			std::memcpy(body.data() + i, str.data(), length);

			header.size = body.size();
		}

		void decodeString(std::string& str, size_t length)
		{
			size_t i = body.size() - length;

			std::memcpy(str.data(), body.data() + i, length);

			body.resize(i);

			header.size = body.size();
		}

		// Pushes data into the message buffer
		template <typename DataType>
		friend message<T>& operator<<(message<T>& msg, const DataType& data)
		{
			// Check that the type of the data being pushed is trivially copyable
			static_assert(std::is_standard_layout<DataType>::value, "Target data is too complex");

			// Cache current size of vector, as this will be the point we insert the data
			size_t i = msg.body.size();

			// Resize the vector by the size of the data being pushed.
			msg.body.resize(i + sizeof(DataType));

			// Physically copy the data into the newly allocated vector space.
			std::memcpy(msg.body.data() + i, &data, sizeof(DataType));

			// Recalculate the message size
			msg.header.size = msg.body.size();

			// Return the target message so it can be "chained".
			return msg;
		}

		// Pops data from the message buffer.
		template<typename DataType>
		friend message<T>& operator>>(message<T>& msg, DataType& data)
		{
			// Check that the type of the data being pushed is trivially copyable
			static_assert(std::is_standard_layout<DataType>::value, "Target data is too complex");
			
			// Cache the location towards the end of the vector where the pulled data starts.
			size_t i = msg.body.size() - sizeof(DataType);
			
			// Physically copy the data from the vector into the user variable.
			std::memcpy(&data, msg.body.data() + i, sizeof(DataType));

			// Shrink the vector to remove read bytes, and reset end position.
			// Making vector smaller doesn't make it reallocate.
			msg.body.resize(i);

			// Recalculate the message size
			msg.header.size = msg.body.size();

			// Return the target message so it can be "chained".
			return msg;
		}
	};

	// Forward declare the connection.
	template<typename T>
	class connection;

	// This specifies which client sended this message.
	template<typename T>
	struct owned_message
	{
		std::shared_ptr<connection<T>> remote = nullptr;
		message<T> msg;

		friend std::ostream& operator<<(std::ostream& os, const owned_message<T>& msg)
		{
			os << msg.msg;
			return os;
		}
	};
}