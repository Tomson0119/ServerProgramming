#include "stdafx.h"
#include "exception.h"

Exception::Exception(const char* msg)
	: errorLog(msg)
{
}

std::string GetLastErrorLog()
{
#ifdef _WIN32
	// 스레드의 마지막 에러를 정수로 받는다.
	DWORD errorID = GetLastError();
	if (errorID == 0) return "";

	// 에러 ID를 포맷에 맞게 문자열로 치환한다.
	LPSTR buffer = nullptr;
	size_t size = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&buffer, 0, NULL);

	// 리턴 값에 옮긴 후 버퍼를 해제한다.
	std::string log(buffer, size);
	LocalFree(buffer);

#else
	std::string log = strerror(errno);
#endif

	return log;
}
