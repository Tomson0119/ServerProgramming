#include "RUDPSocket.h"
#include <iostream>
#include <exception>

using namespace std::chrono_literals;

RUDPSocket::RUDPSocket(const SckType& type)
	: hostEp{}, loop{ true }
{
	sckHandle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (sckHandle == INVALID_SOCKET)
		throw std::exception("Faild to create handle");

	memset(recvBuffer, 0, BUF_SIZE);
	sendThread = std::thread{ sendThreadFunc, std::ref(*this) };
}

RUDPSocket::~RUDPSocket()
{
	loop = false;

	if (sendThread.joinable())
		sendThread.join();

	if (sckHandle)
		closesocket(sckHandle);
}

void RUDPSocket::connect()
{
	if (::connect(sckHandle, reinterpret_cast<sockaddr*>(&hostEp), sizeof(hostEp)) == SOCKET_ERROR)
		throw std::exception("Failed to connect");
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

bool RUDPSocket::getFrontMsg(message& msg)
{
	std::unique_lock<std::mutex> lock{ mainQueMut };
	if (mainSendQueue.empty()) return false;
	
	msg = mainSendQueue.front();
	return true;
}

void RUDPSocket::printEndpoint()
{
	sockaddr_in sckAddr;
	int len = sizeof(sckAddr);
	getsockname(sckHandle, reinterpret_cast<sockaddr*>(&sckAddr), &len);

	printIPAndPort(sckAddr);
}

void RUDPSocket::pushToSendBuffer(std::byte* packet, size_t pckSize, sockaddr_in& hostEp)
{
	message msg;
	msg.peer = getPeerKey(hostEp);

	msg.packet.resize(pckSize);
	memcpy_s(msg.packet.data(), pckSize, packet, pckSize);

	std::unique_lock<std::mutex> lock{ mainQueMut };
	mainSendQueue.push(msg);
}

void RUDPSocket::sendTo(std::byte* packet, size_t pckSize, sockaddr_in& hostEp)
{
	if (sendto(sckHandle, reinterpret_cast<const char*>(packet), pckSize, 0,
		reinterpret_cast<sockaddr*>(&hostEp), sizeof(hostEp)) == SOCKET_ERROR)
	{
		throw std::exception("Failed to send to: " + WSAGetLastError());
	}
}

void RUDPSocket::recvFrom(std::byte* packet, size_t bufSize, sockaddr_in& senderEp)
{
	int fromLen = sizeof(senderEp);

	if (recvfrom(sckHandle, reinterpret_cast<char*>(packet), bufSize, 0,
		reinterpret_cast<sockaddr*>(&senderEp), &fromLen) == SOCKET_ERROR)
	{
		throw std::exception("Failed to recv from: " + WSAGetLastError());
	}
}

void RUDPSocket::printIPAndPort(const sockaddr_in& addr)
{
	const int MAX_IP_LEN = 256;
	char ip[MAX_IP_LEN];
	inet_ntop(AF_INET, &addr.sin_addr, ip, MAX_IP_LEN);

	std::cout << "IP: " << ip << "\n";
	std::cout << "Port: " << ntohs(addr.sin_port) << "\n";
}

std::string RUDPSocket::getPeerKey(const sockaddr_in& addr)
{
	const int MAX_IP_LEN = 256;
	char ip[MAX_IP_LEN];
	inet_ntop(AF_INET, &addr.sin_addr, ip, MAX_IP_LEN);

	std::string ipStr = ip;
	u_short port = ntohs(addr.sin_port);

	return ipStr + ':' + std::to_string(port);
}

sockaddr_in RUDPSocket::stringToEp(const std::string& peer)
{
	int split = 0;
	for (int i = 0; i < peer.size(); i++)
	{
		if (peer[i] == ':')
			split = i;
	}

	std::string ip = peer.substr(0, split);
	u_short port = std::stoi(peer.substr(split+1));

	sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);

	return addr;
}

void RUDPSocket::sendThreadFunc(RUDPSocket& sck)
{
	while (sck.loop)
	{
		message msg{};
		while (sck.getFrontMsg(msg))
		{
			auto hostEp = stringToEp(msg.peer);
			sck.sendTo(msg.packet.data(), msg.packet.size(), hostEp);
		}
		std::this_thread::sleep_for(10ms);
	}
}