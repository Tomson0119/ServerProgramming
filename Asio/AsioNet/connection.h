#pragma once

#include "common.h"
#include "tsqueue.h"
#include "message.h"

namespace net
{
	// This class is managed by shared_ptr 
	// and enable_shared_from_this returns new shared_ptr 
	// when same object is called to create shared_ptr
	// and prevent destroying object that is already destroyed.
	template<typename T>
	class connection : public std::enable_shared_from_this<connection<T>>
	{
	public:
		connection() { }

		virtual ~connection() { }

		bool ConnectToServer();
		bool Disconnect();
		bool IsConnected() const;

		bool Send(const message<T>& msg);

	protected:
		// Each connection has a unique socket to a remote.
		asio::ip::tcp::socket m_socket;

		// This context is shared with the whole asio instance
		// like server can have multiple connection, but only one context.
		asio::io_context& m_context;

		// This queue holds all messages to be sent to the remote side
		// of this connection.
		tsqueue<message<T>> m_messagesOut;

		// This queue holds all messages that have been received from
		// the remote side of this connection. Note it is a reference
		// as the "owner" of this connection is expected to provide a queue.
		tsqueue<owned_message>& m_messagesIn;

	};
}