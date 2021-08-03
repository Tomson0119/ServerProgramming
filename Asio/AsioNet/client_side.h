#pragma once

#include "common.h"
#include "tsqueue.h"
#include "message.h"
#include "connection.h"

namespace net
{
	template<typename T>
	class client_side
	{
	public:
		client_side()
		{

		}

		virtual ~client_side()
		{
			// If client is destroyed, always disconnect from server
			Disconnect();
		}

		// Connect to server with host/ip and port
		bool Connect(const std::string& host, const uint16_t port)
		{
			try 
			{
				// Resolve hostname/ip-address into physical address
				asio::ip::tcp::resolver resolver(m_context);
				asio::ip::tcp::resolver::results_type endpoint 
					= resolver.resolve(host, std::to_string(port));

				// Create a connection
				m_connection = std::make_unique<connection<T>>(
					connection<T>::owner::client,
					m_context, asio::ip::tcp::socket(m_context), m_messagesIn);

				// Connection object connects to server.
				m_connection->ConnectToServer(endpoint);

				// Start context thread.
				m_thrContext = std::thread([this]() { m_context.run(); });
			}
			catch (std::exception& ex)
			{
				std::cerr << "Connect failed: " << ex.what() << '\n';
				return false;
			}
			return true;
		}

		// Disconnect from server
		void Disconnect()
		{
			if (IsConnected())	// If connected, disconnect it.
			{
				m_connection->Disconnect();
			}

			// Stop the context and join thread.
			m_context.stop();
			if (m_thrContext.joinable())
				m_thrContext.join();

			// Destroy the connection object.
			m_connection.release();
		}

		// Check if client is actually connected to a server
		bool IsConnected()
		{
			if (m_connection)
				return m_connection->IsConnected();
			else
				return false;
		}

		// Send message to server
		void Send(const message<T>& msg)
		{
			if (IsConnected())
				m_connection->Send(msg);
		}

		// Retrieve queue of messages from server
		tsqueue<owned_message<T>>& Incoming()
		{
			return m_messagesIn;
		}

	protected:
		// asio context handles the data transfer.
		asio::io_context m_context;

		// thread of its own to excute its work commands.
		std::thread m_thrContext;

		// Client has single instance of connection
		std::unique_ptr<connection<T>> m_connection;

	private:
		// Thread safe queue of messages from the server
		tsqueue<owned_message<T>> m_messagesIn;
		
	};
}