#include "stdafx.h"
#include "WSAInit.h"

WSAInit::WSAInit()
{
	initResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
}

WSAInit::~WSAInit()
{
	WSACleanup();
}

bool WSAInit::Init()
{
	return (initResult == 0);
}
