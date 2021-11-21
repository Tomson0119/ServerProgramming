#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "MSWSock.lib")

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <exception>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <mutex>

#include "NetException.h"
#include "Protocol.h"

typedef unsigned char uchar;