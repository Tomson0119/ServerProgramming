#include "RUDPSocket.h"
#include <iostream>
#include <string>
#include <vector>

WSAData wsaData;
const u_short MY_PORT = 5505;

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

		uint64_t prev_val = 0;
		uint64_t drops = 0;
		uint64_t hit_cnt = 0;
		while (prev_val < 15'000)
		{
			netSck.recvFrom();
			uint64_t val = netSck.GetLastValue();

			if (val > 0 && prev_val + 1 != val)
			{
				drops += (val - prev_val - 1);
			}
			hit_cnt += 1;
			prev_val = val;
			std::cout << val << "\n";
		}

		RUDPSocket::printIPAndPort(senderEp);
		
		double drop_rate = (double)drops / (prev_val + 1);
		double hit_rate = (double)hit_cnt / (prev_val + 1);
		std::cout << "Drop rate: " << drop_rate * 100 << "% (" << drops << ")\n";
		std::cout << "Hit rate: " << hit_rate * 100 << "% (" << hit_cnt << ")\n";
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what() << "\n";
	}
	WSACleanup();
}