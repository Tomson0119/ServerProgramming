#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")


#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <exception>

#include "NetException.h"

using ushort_t = unsigned short;