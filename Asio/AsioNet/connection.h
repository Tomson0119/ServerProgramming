#pragma once

#include "common.h"
#include "tsqueue.h"
#include "message.h"

namespace net
{
	// Forward declare
	template<typename T>
	class server_side;

	// This class is managed by shared_ptr 
	// and enable_shared_from_this returns new shared_ptr 
	// when same object is called to create shared_ptr
	// and prevent destroying object that is already destroyed.
	template<typename T>
	class connection : public std::enable_shared_from_this<connection<T>>
	{
	public:
		// A connection is 'owned' by either server or client,
		// its behavior is slightly different between the two
		enum class owner
		{
			server,
			client
		};

	public:
		// Specify owner, connect to context,
		// transfer the socket, provide reference to message queue
		connection(owner parent, asio::io_context& context,
			asio::ip::tcp::socket socket, tsqueue<owned_message<T>>& qIn)
			: m_context(context), m_socket(std::move(socket)), m_messagesIn(qIn)
		{
			m_ownerType = parent;

			// Construct validation check data
			if (m_ownerType == owner::server)
			{
				// Generate random data for the client
				// to transform and send back for validation.
				m_handshakeOut = uint64_t(std::chrono::system_clock::now().time_since_epoch().count());

				// Pre-calculate the result for checking when the client responds.
				m_handshakeCheck = scramble(m_handshakeOut);
			}
			else
			{
				m_handshakeOut = 0;
				m_handshakeCheck = 0;
			}			
		}

		virtual ~connection() { }

		uint32_t GetID() const
		{
			return id;
		}

		void ConnectToClient(net::server_side<T>* server, uint32_t uid = 0)
		{
			if (m_ownerType == owner::server)
			{
				if (IsConnected()) {
					id = uid;

					// ReadHeader();

					// Write out the handshake data
					WriteValidation();

					// Next, wait asynchronously for validation data
					// sent from the client
					ReadValidation(server);
				}
			}
		}

		void ConnectToServer(const asio::ip::tcp::resolver::results_type& endpoint)
		{
			// Only clients can connect to server
			if (m_ownerType == owner::client)
			{
				// Request asio attempts to connect to an endpoint.
				asio::async_connect(m_socket, endpoint,
					[this](std::error_code ec, asio::ip::tcp::endpoint ep)
					{
						// If successfully connected, assign
						// reading message task right after.
						if (!ec)
						{
							// ReadHeader();

							// First thing server will send is packet to be validated.
							// so wait for that and respond.
							ReadValidation();
						}
					});
			}
		}

		void Disconnect()
		{
			if (IsConnected())
				// we post a job to context, not immediately close socket,
				// because context may still working and we need to wait 
				// until context thread join.
				asio::post(m_context, [this]() { m_socket.close(); });
		}

		bool IsConnected() const
		{
			return m_socket.is_open();
		}

	public:
		void Send(const message<T>& msg)
		{
			// Send a job(lambda func) to context.
			asio::post(m_context,
				[this, msg]()
				{
					// if queue is empty,	 bWritingMessage is false
					// if queue isn't empty, bWritingMessage is true
					bool bWritingMessage = !m_messagesOut.empty();
					m_messagesOut.push_back(msg);

					// if queue WAS empty,
					// that means async_write isn't running right now.
					// otherwise, async_write is up and running, 
					// so we don't need to call this function again.
					if(!bWritingMessage)
						WriteHeader();
				});
		}

	private:
		// ASYNC - Prime context ready to read a message header
		void ReadHeader()
		{
			asio::async_read(m_socket,
				asio::buffer(&m_msgTempIn.header, sizeof(message_header<T>)),
				[this](std::error_code ec, std::size_t size)
				{
					if (!ec)
					{
						if (m_msgTempIn.header.size > 0)
						{
							// resize buffer based on size we get advance.
							m_msgTempIn.body.resize(m_msgTempIn.header.size);
							ReadBody();
						}
						else
						{
							// If there's no data to read,
							// finish reading and add message to a queue.
							AddIncomingMessageQueue();
						}
					}
					else
					{
						std::cout << "[" << id << "] Read header failed: \n";
						m_socket.close();
					}
				});
		}

		// ASYNC - Prime context ready to read a message body
		void ReadBody()
		{
			asio::async_read(m_socket,
				asio::buffer(m_msgTempIn.body.data(), m_msgTempIn.body.size()),
				[this](std::error_code ec, std::size_t size)
				{
					if (!ec)
					{
						// If read success, add message to the queue.
						AddIncomingMessageQueue();
					}
					else
					{
						std::cout << "[" << id << "] Read body failed.\n";
						m_socket.close();
					}
				});
		}

		// ASYNC - Prime context to write a message header
		void WriteHeader()
		{
			// Read from messages out buffer and send it to connected object.
			asio::async_write(m_socket,
				asio::buffer(&m_messagesOut.front().header, sizeof(message_header<T>)),
				[this](std::error_code ec, std::size_t size)
				{
					if (!ec)
					{
						if (m_messagesOut.front().body.size() > 0)
						{
							// If body has some message, send body message.
							WriteBody();
						}
						else
						{
							// If there's no message, just delete it.
							m_messagesOut.pop_front();

							// If messages are still exist,
							// write again.
							if (!m_messagesOut.empty())
								WriteHeader();
						}
					}
					else
					{
						std::cout << "[" << id << "] Write header failed.\n";
						m_socket.close();
					}
				});
		}

		// ASYNC - Prime context to write a message body
		void WriteBody()
		{
			asio::async_write(m_socket,
				asio::buffer(m_messagesOut.front().body.data(), m_messagesOut.front().body.size()),
				[this](std::error_code ec, std::size_t size)
				{
					if (!ec)
					{
						// finished message write,
						// so remove from queue.
						m_messagesOut.pop_front();

						// write again if queue is not empty
						if (!m_messagesOut.empty())
							WriteHeader();
					}
					else
					{
						std::cout << "[" << id << "] Write body failed.\n";
						m_socket.close();
					}
				});
		}

		void AddIncomingMessageQueue()
		{
			if (m_ownerType == owner::server)
				m_messagesIn.push_back({ this->shared_from_this(), m_msgTempIn });
			else
				// Client has only one connection anyway.
				m_messagesIn.push_back({ nullptr, m_msgTempIn });
			
			// Wait and read another header
			ReadHeader();
		}

		// Encrypt data
		uint64_t scramble(uint64_t nInput)
		{
			uint64_t out = nInput ^ 0xDEADBEEFC0DECAFE;
			out = (out & 0xF0F0F0F0F0F0F0) >> 4 | (out & 0x0F0F0F0F0F0F0F) << 4;
			return out ^ 0xC0DEFACE12345678;
		}

		// ASYNC - Used by both client and server to write validation packet.
		void WriteValidation()
		{
			asio::async_write(m_socket, asio::buffer(&m_handshakeOut, sizeof(uint64_t)),
				[this](std::error_code ec, std::size_t size)
				{
					if (!ec)
					{
						// Validation data sent, 
						// clients should wait for a response
						if (m_ownerType == owner::client)
							ReadHeader();
					}
					else
					{
						std::cout << "Write validation failed.\n";
						m_socket.close();
					}
				});
		}

		void ReadValidation(net::server_side<T>* server = nullptr)
		{
			asio::async_read(m_socket, asio::buffer(&m_handshakeIn, sizeof(uint64_t)),
				[this, server](std::error_code ec, std::size_t size)
				{
					if (!ec)
					{
						if (m_ownerType == owner::server)
						{
							if (m_handshakeIn == m_handshakeCheck)
							{
								// Client has sent valid solutioin,
								// so allow it to connect.
								std::cout << "Client Validated\n";
								server->OnClientValidated(this->shared_from_this());

								// Waiting to receive data now
								ReadHeader();
							}
							else
							{
								// Client gave incorrect data, so disconnect.
								std::cout << "Client Disconnected (Fail Validation)\n";
								m_socket.close();
							}
						}
						else
						{
							// Connection is a client, so solve puzzle.
							m_handshakeOut = scramble(m_handshakeIn);

							// Write back to server.
							WriteValidation();
						}
					}
					else
					{
						std::cout << "Client Disconnected (ReadValidation)\n";
						m_socket.close();
					}
				});
		}

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
		tsqueue<owned_message<T>>& m_messagesIn;
		message<T> m_msgTempIn;

		// The 'owner' decides how some of the connection behaves.
		owner m_ownerType = owner::server;
		uint32_t id = 0;

		// Handshake Validation
		uint64_t m_handshakeOut = 0;
		uint64_t m_handshakeIn = 0;
		uint64_t m_handshakeCheck = 0;
	};
}