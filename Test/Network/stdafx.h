#pragma once

#ifdef _WIN32
#include <WinSock2.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#else
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <errno.h>
#include <string.h>
#include <exception>
#include <string>
#include <sstream>
#include <cstdint>


#ifdef _WIN32
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "mswsock.lib")
#endif