#include "stdafx.h"
#include "exception.h"

Exception::Exception(const char* msg)
	: errorLog(msg)
{
}

std::string GetLastErrorLog()
{
#ifdef _WIN32
	// �������� ������ ������ ������ �޴´�.
	DWORD errorID = GetLastError();
	if (errorID == 0) return "";

	// ���� ID�� ���˿� �°� ���ڿ��� ġȯ�Ѵ�.
	LPSTR buffer = nullptr;
	size_t size = FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&buffer, 0, NULL);

	// ���� ���� �ű� �� ���۸� �����Ѵ�.
	std::string log(buffer, size);
	LocalFree(buffer);

#else
	std::string log = strerror(errno);
#endif

	return log;
}
