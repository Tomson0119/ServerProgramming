#include "stdafx.h"
#include "NetException.h"

NetException::NetException(const std::string& info)
{
	LPSTR msgBuffer{};
	size_t size = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&msgBuffer, 0, NULL);
	mErrorString = info + ": " + msgBuffer;
	LocalFree(msgBuffer);
}

NetException::~NetException()
{
}

const char* NetException::what() const
{
	return mErrorString.c_str();
}
