#include "stdafx.h"

D2DException::D2DException(const std::string& errorString)
{
	LPSTR msgBuffer = nullptr;
	size_t size = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&msgBuffer, 0, NULL);
	mErrorString = errorString + " : " + msgBuffer;
	LocalFree(msgBuffer);
}

const char* D2DException::what() const
{
	return mErrorString.c_str();
}


