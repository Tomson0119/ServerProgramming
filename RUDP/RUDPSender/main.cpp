#include "RUDPSocket.h"
#include <iostream>
#include <string>
#include <numeric>
#include <thread>
#include <chrono>

using namespace std::chrono_literals;

WSAData wsaData;
const u_short MY_PORT = 53800;

const std::string HOST_IP = "127.0.0.1";
const u_short HOST_PORT = 53801;

int main()
{
	try 
	{
		if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		{
			throw std::exception("Failed to init WSAData");
		}
		RUDPSocket netSck(RUDPSocket::SckType::UDP);
		netSck.bind(MY_PORT);
		netSck.printEndpoint();
		netSck.setHostEndpoint(HOST_IP, HOST_PORT);

		uint64_t val = 0;
		while (val < 16'000)
		{
			netSck.sendTo(reinterpret_cast<std::byte*>(&val), sizeof(val));
			val += 1;
		}
	}
	catch (std::exception& ex) 
	{
		std::cout << ex.what() << "\n";
	}
	WSACleanup();
}