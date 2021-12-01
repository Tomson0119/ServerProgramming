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

using namespace std::chrono_literals;

typedef unsigned char uchar;

struct PlayerInfo
{
	int id;
	short x;
	short y;
	char message[MAX_CHAT_SIZE];
	char name[MAX_NAME_SIZE];
};
