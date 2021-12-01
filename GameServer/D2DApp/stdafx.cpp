#include "stdafx.h"

D2DException::D2DException(const std::wstring& info)
{
	LPWSTR msgBuffer{};
	size_t size = FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, WSAGetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&msgBuffer, 0, NULL);
	mErrorString = info + L": " + msgBuffer;
	LocalFree(msgBuffer);
}

D2DException::D2DException(HRESULT hr, const std::wstring& functionName, const std::wstring& fileName, int lineNumber)
	: mError(hr), mFuncName(functionName), mFileName(fileName), mLineNumber(lineNumber)
{
	_com_error error(mError);
	std::wstring msg = error.ErrorMessage();
	mErrorString = mFuncName + L" failed in " + mFileName + L";  line "
		+ std::to_wstring(mLineNumber) + L";  Error : " + msg;
}

D2DException::~D2DException()
{
}

const wchar_t* D2DException::what() const
{
	return mErrorString.c_str();
}


