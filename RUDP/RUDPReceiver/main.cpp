#include "RUDPSocket.h"
#include <iostream>
#include <string>
#include <vector>

WSAData wsaData;
const u_short MY_PORT = 53801;

const std::string HOST_IP = "127.0.0.1";
const u_short HOST_PORT = 53800;

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

		const int BUF_SIZE = 128;
		std::byte packet[BUF_SIZE];

		uint64_t prev_val = 0;
		uint64_t drops = 0;
		uint64_t hit_cnt = 0;
		while (prev_val < 15'000)
		{
			netSck.recvFrom(packet, BUF_SIZE);
			uint64_t val = *reinterpret_cast<uint64_t*>(packet);
			
			if (val > 0 && prev_val + 1 != val)
			{
				drops += (val - prev_val - 1);
			}
			hit_cnt += 1;
			prev_val = val;
			std::cout << val << "\n";
		}

		double drop_rate = (double)drops / (prev_val + 1);
		double hit_rate = (double)hit_cnt / (prev_val + 1);
		std::cout << "Drop rate: " << drop_rate * 100 << "(" << drops << ")\n";
		std::cout << "Hit rate: " << hit_rate * 100 << "(" << hit_cnt << ")\n";
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what() << "\n";
	}
	WSACleanup();
}