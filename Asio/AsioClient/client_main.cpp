
#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif

#define ASIO_STANDALONE

#include <asio.hpp>
#include <asio/ts/buffer.hpp>
#include <asio/ts/internet.hpp>

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

using namespace std;

constexpr int Local_port = 5505;
constexpr int http_port = 80;

vector<char> vBuffer(20 * 1024);

int translate_dns(const string& domain, short port);

void GrabSomeData(asio::ip::tcp::socket& socket)
{
	// Asynchronous function shows anything right away.
	// so we need to pass a lambda function which describe what to do. 
	socket.async_read_some(asio::buffer(vBuffer.data(), vBuffer.size()),
		[&](error_code ec, size_t length)
		{
			if (!ec)
			{
				cout << "\n\nRead " << length << " bytes\n\n";
				for (int i = 0; i < length; ++i)
					cout << vBuffer[i];

				// If there's more data to read, read it recursively.
				// It doesn't cause recursive nightmare.
				// because async_read_some() will stop,
				// when there's nothing to read anymore.
				GrabSomeData(socket);
			}
		});
}

int main()
{
	// variable contains error code.
	asio::error_code ec;

	// Create a context, essentially the platform specific interface.
	asio::io_context context;

	// Give some fake work to asio so the context doesn't exit immediately
	asio::io_context::work idleWork(context);
	
	// Start the context with non-blocking way.
	// this ends when context runs out of work to do.
	thread thrContext = thread([&]() { context.run(); });

	// Create a endpoint we want to connect to
	asio::ip::tcp::endpoint endpoint(asio::ip::make_address("51.38.81.49", ec), http_port);

	// Create a tcp socket, the context will deliver the implementation.
	asio::ip::tcp::socket socket(context);

	socket.connect(endpoint, ec);

	if (!ec)
		cout << "Connected" << endl;
	else
		cout << "Failed to connect to address: " << ec.message() << endl;

	if (socket.is_open())
	{
		GrabSomeData(socket);

		string request =
			"GET /index.html HTTP/1.1\r\n"
			"Host: example.com\r\n"
			"Connection: close\r\n\r\n";

		// Try to send data as much as possible.
		// buffer contains data bytes and size.
		socket.write_some(asio::buffer(request.data(), request.size()), ec);

		//// Since write_some function is non-blocking.
		//// It needs some time for server to receive requested data.
		//

		//// Block socket until it is ready to read
		//socket.wait(socket.wait_read);

		//// bytes available is different everytime we connect, because we can't tell
		//// when we'll be available, when transporting is done..
		//size_t bytes = socket.available();
		//cout << "Bytes available: " << bytes << endl << endl;

		//if (bytes > 0)
		//{
		//	// Read server responses into buffer array.
		//	socket.read_some(asio::buffer(vBuffer.data(), vBuffer.size()), ec);

		//	for (auto c : vBuffer)
		//		cout << c;
		//}

		// Program does something else, while asio handles data transfer in background.
		this_thread::sleep_for(2000ms);

		context.stop();
		if (thrContext.joinable()) thrContext.join();
	}
}

int translate_dns(const string& domain, short port)
{
	asio::io_service is;
	asio::ip::tcp::resolver::query q(domain, to_string(port), asio::ip::tcp::resolver::query::numeric_service);
	asio::ip::tcp::resolver resolver(is);
	asio::error_code ec;

	auto iter = resolver.resolve(q, ec);
	if (ec.value()) {
		cerr << "error: " << ec.value() << ", msg: " << ec.message() << endl;
		return ec.value();
	}

	while (iter != asio::ip::tcp::resolver::iterator())
	{
		auto ep = iter->endpoint();
		cout << ep.address().to_string() << endl;
		iter++;
	}
	return 0;
}
