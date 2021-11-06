#include "stdafx.h"
#include "NetException.h"

NetException::NetException()
{
	LPSTR msgBuffer{};
	size_t size = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&msgBuffer, 0, NULL);
	m_errorString = msgBuffer;
	LocalFree(msgBuffer);
}

NetException::~NetException()
{
}

const char* NetException::what() const
{
	return m_errorString.c_str();
}
