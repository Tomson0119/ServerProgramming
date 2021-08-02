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
		client_side() : m_socket(m_context)
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
				// Create a connection
				m_connection = std::make_unique<connection<T>>();

				// Resolve hostname/ip-address into physical address
				asio::ip::tcp::resolver resolver(m_context);
				m_endpoint = resolver.resolve(host, std::to_string(port));

				// Connection object connects to server.
				m_connection->ConnectToServer(m_endpoint);

				// Start context thread.
				m_thrContext = std::thread([this]() { m_context.run(); });
			}
			catch (std::exception& ex)
			{
				std::cerr << "Connect failed: " << e.what() << '\n';
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

		// Socket connected to the server
		asio::ip::tcp::socket m_socket;

		// endpoint of the server
		asio::ip::tcp::endpoint m_endpoint;

		// Client has single instance of connection
		std::unique_ptr<connection<T>> m_connection;

	private:
		// Thread safe queue of messages from the server
		tsqueue<owned_message<T>> m_messagesIn;
		
	};
}