#include "RUDPSocket.h"
#include <iostream>
#include <exception>

RUDPSocket::RUDPSocket(const SckType& type)
	: hostEp{}
{
	if (type == SckType::TCP)
	{
		sckHandle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	}
	else
	{
		sckHandle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	}

	if (sckHandle == INVALID_SOCKET)
		throw std::exception("Faild to create handle");
}

RUDPSocket::~RUDPSocket()
{
	if (sckHandle)
		closesocket(sckHandle);
}

void RUDPSocket::bind(u_short port)
{
	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (sckHandle)
	{
		int res = ::bind(sckHandle, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
		if (res != 0)
		{
			throw std::exception("Faild to bind socket");
		}
	}
}

void RUDPSocket::printEndpoint()
{
	sockaddr_in sckAddr;
	int len = sizeof(sckAddr);
	getsockname(sckHandle, reinterpret_cast<sockaddr*>(&sckAddr), &len);

	const int MAX_IP_LEN = 256;
	char ip[MAX_IP_LEN];
	inet_ntop(AF_INET, &sckAddr.sin_addr, ip, MAX_IP_LEN);

	std::cout << "IP: " << ip << "\n";
	std::cout << "Port: " << ntohs(sckAddr.sin_port) << "\n";
}

void RUDPSocket::setHostEndpoint(const std::string& ip, const u_short port)
{
	hostEp.sin_family = AF_INET;
	hostEp.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &hostEp.sin_addr);
}

void RUDPSocket::sendTo(std::byte* packet, size_t pckSize)
{
	if (sendto(sckHandle, reinterpret_cast<const char*>(packet), pckSize, 0,
		reinterpret_cast<sockaddr*>(&hostEp), sizeof(hostEp)) == SOCKET_ERROR)
	{
		throw std::exception("Failed to send: " + WSAGetLastError());
	}
}

void RUDPSocket::recvFrom(std::byte* packet, size_t bufSize)
{
	int fromLen = sizeof(hostEp);
	if (recvfrom(sckHandle, reinterpret_cast<char*>(packet), bufSize, 0,
		reinterpret_cast<sockaddr*>(&hostEp), &fromLen) == SOCKET_ERROR)
	{
		throw std::exception("Failed to recv");
	}
}
