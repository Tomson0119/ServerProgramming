#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "MSWSock.lib")

extern "C" {
#include "include/lua.h"
#include "include\lauxlib.h"
#include "include\lualib.h"
}
#pragma comment (lib, "lua54.lib")

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <exception>
#include <array>
#include <mutex>
#include <thread>
#include <atomic>
#include <unordered_map>
#include <unordered_set>
#include <concurrent_priority_queue.h>

#include "NetException.h"
#include "Protocol.h"

#define ABS(x) ((x < 0) ? -(x) : (x))

using namespace std::chrono_literals;

struct PlayerInfo
{
	char name[MAX_NAME_SIZE];
	std::atomic<short> level;
	std::atomic<short> x;
	std::atomic<short> y;
	std::atomic<short> hp;
	std::atomic<short> max_hp;
	std::atomic<int> exp;

	PlayerInfo() : 
		level{ -1 },
		x{ -1 },
		y{ -1 },
		hp{ -1 },
		max_hp{ -1 },
		exp{ -1 }
	{
		memset(name, 0, MAX_NAME_SIZE);
	}

	PlayerInfo(const PlayerInfo& other) :
		level{ other.level.load() },
		x{ other.x.load() },
		y{ other.y.load() },
		hp{ other.hp.load() },
		max_hp{ other.max_hp.load() },
		exp{ other.exp.load() }
	{
		strncpy_s(name, other.name, MAX_NAME_SIZE);
	}

	PlayerInfo& operator=(const PlayerInfo& other)
	{
		if (this != &other)
		{
			level = other.level.load();
			x = other.x.load();
			y = other.y.load();
			hp = other.hp.load();
			max_hp = other.max_hp.load();
			exp = other.exp.load();
			strncpy_s(name, other.name, MAX_NAME_SIZE);
		}
		return *this;
	}
};

inline char GetPacketType(std::byte* pck)
{
	return *reinterpret_cast<char*>(pck + sizeof(unsigned char));
}

