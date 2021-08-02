#pragma once

#include "common.h"
#include "message.h"
#include "connection.h"
#include "tsqueue.h"

namespace net
{
	template<typename T>
	class server_side
	{
	public:
		server_side(uint16_t port)
			// Acceptor will listen to all IPv4 connection from outside
			// when we start server.
			: m_acceptor(m_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
		{

		}

		virtual ~server_side()
		{
			Stop();
		}

		bool Start()
		{
			try
			{
				// Acceptor listens to connection asynchronously.
				WaitForClientConnection();

				m_thrContext = std::thread([this]() { m_context.run(); });
			}
			catch (std::exception& ex)
			{
				std::cerr << "[SERVER] Start failed: " << ex.what() << '\n';
				return false;
			}

			std::cout << "[SERVER] Started!\n";
			return true;
		}

		void Stop()
		{
			m_context.stop();  // Close the context.

			// Wait for thread to terminate.
			if (m_thrContext.joinable())
				m_thrContext.join();

			std::cout << "[SERVER] Stopped!\n";
		}

		// ASYNC - Instruct asio to wait for connection.
		void WaitForClientConnection()
		{
			m_acceptor.async_accept(
				[this](std::error_code ec, asio::ip::tcp::socket socket)
				{
					if (!ec)
					{
						// This prints out sockets ip address
						std::cout << "[SERVER] New connection: " << socket.remote_endpoint() << '\n';

						std::shared_ptr<connection<T>> newConn =
							std::make_shared<connection<T>>(connection<T>::owner::server, 
								m_context, std::move(socket), m_messagesIn);

						// Give the user server a chance to deny connection.
						if (OnClientConnect(newConn))
						{
							// Connection allowed, so add to container of new connections
							m_connections.push_back(std::move(newConn));

							m_connections.back()->ConnectToClient(nIDCounter++);

							std::cout << "[" << m_connections.back()->GetID() << "] Connection approved\n";
						}
						else
						{
							std::cout << "[-----] Connection denied\n";
						}
					}
					else
					{
						std::cerr << "[SERVER] New connection error: " << ec.message() << '\n';
					}

					// Prime the asio context with more task.
					// simply wait for another connection.
					WaitForClientConnection();
				});
		}

		// Send a message to a specific client
		void MessageClient(std::shared_ptr<connection<T>> client, const message<T>& msg)
		{
			if (client && client->IsConnected())
			{
				client->Send(msg);
			}
			else
			{
				OnClientDisconnect(client);
				client.reset();

				m_connections.erase(std::ranges::remove(m_connections, client), m_connections.end());
			}
		}

		// Send a message to all clients.
		void MessageAllClients(const message<T>& msg, std::shared_ptr<connection<T>> ignoreClient = nullptr)
		{
			bool bInvalidClientExists = false;

			for (auto& client : m_connections)
			{
				if (client && client->IsConnected())
				{
					if (client != ignoreClient)
						client->Send(msg);
				}
				else
				{
					// The client couldn't be contacted.
					// So assume it has disconnected.
					OnClientDisconnect(client);
					client.reset();
					bInvalidClientExists = true;
				}
			}

			if (bInvalidClientExists)
			{
				// If invalid client exits, remove that invaild client.
				m_connections.erase(std::ranges::remove(m_connections, nullptr), m_connections.end());
			}
		}

		void Update(size_t nMaxMessages = -1)
		{
			size_t nMessageCount = 0;
			while (nMessageCount < nMaxMessages && !m_messagesIn.empty())
			{
				// Grab the front message
				auto msg = m_messagesIn.pop_front();

				// Pass to message handler
				OnMessage(msg.remote, msg.msg);

				nMessageCount++;
			}
		}

	protected:
		// Called when a client connects, you can veto the connection by returning false
		virtual bool OnClientConnect(std::shared_ptr<connection<T>> client)
		{
			return false;
		}

		// Called when a client appears to have disconnected
		virtual void OnClientDisconnect(std::shared_ptr<connection<T>> client)
		{

		}

		// Called when a message arrives
		virtual void OnMessage(std::shared_ptr<connection<T>> client, message<T>& msg)
		{

		}

	protected:
		// Order of declaration is important
		asio::io_context m_context;
		std::thread m_thrContext;

		// Accepts client connection.
		asio::ip::tcp::acceptor m_acceptor;

		// Client will be identified in the wider system via an ID
		// It is much simpler than IP addresses and ports.
		uint32_t nIDCounter = 1000;

		// tsqueue for incoming message packets.
		tsqueue<owned_message<T>> m_messagesIn;

		// Container of active validated connections.
		std::deque<std::shared_ptr<connection<T>>> m_connections;
	};
}