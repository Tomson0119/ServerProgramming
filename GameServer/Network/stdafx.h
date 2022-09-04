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
	short level;
	short x;
	short y;
	short hp;
	short max_hp;
	int exp;
};

inline char GetPacketType(std::byte* pck)
{
	return *reinterpret_cast<char*>(pck + sizeof(unsigned char));
}

