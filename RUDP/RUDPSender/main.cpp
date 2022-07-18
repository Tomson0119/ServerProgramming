#include "RUDPSocket.h"
#include <iostream>
#include <string>
#include <numeric>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

WSAData wsaData;
//const u_short MY_PORT = 53800;

const std::string HOST_IP = "192.168.0.83";
const u_short HOST_PORT = 5505;

int main()
{
	try 
	{
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			throw std::exception("Failed to init WSAData");
		}
		RUDPSocket netSck(RUDPSocket::SckType::UDP);
		netSck.bind(0);
		netSck.printEndpoint();

		sockaddr_in hostEp;
		hostEp.sin_family = AF_INET;
		hostEp.sin_port = htons(HOST_PORT);
		inet_pton(AF_INET, HOST_IP.c_str(), &hostEp.sin_addr);

		uint64_t val = 0;
		while (val < 16'000)
		{
			netSck.pushToSendBuffer(reinterpret_cast<std::byte*>(&val), sizeof(val), hostEp);
			
			val += 1;
		}
	}
	catch (std::exception& ex) 
	{
		std::cout << ex.what() << "\n";
	}
	WSACleanup();
}