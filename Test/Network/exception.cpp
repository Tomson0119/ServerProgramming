#include "stdafx.h"
#include "exception.h"

Exception::Exception(const char* msg)
	: errorLog(msg)
{
}

std::string GetLastErrorMessage()
{
	// Get error message.
	DWORD errorMessage = GetLastError();
	if (errorMessage == 0)
		return "";

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessage, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&messageBuffer, 0, NULL);

	std::string message(messageBuffer, size);

	LocalFree(messageBuffer);

	return message;
}
