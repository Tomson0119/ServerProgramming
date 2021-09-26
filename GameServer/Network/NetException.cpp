#include "stdafx.h"
#include "NetException.h"

NetException::NetException()
{
	if (WSAGetLastError() == 0){
		MessageBox(NULL, L"Hello", L"Please don't", MB_OK);
		return;
	}

	LPWSTR msgBuffer{};
	size_t size = FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&msgBuffer, 0, NULL);
	errorInfo.assign(msgBuffer, size);
	LocalFree(msgBuffer);
}

NetException::~NetException()
{
}

const char* NetException::what() const
{
	char msg[1024];
	int len = WideCharToMultiByte(CP_ACP, 0, errorInfo.c_str(), -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, errorInfo.c_str(), -1, msg, len, NULL, NULL);
	return msg;
}
