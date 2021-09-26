#include "stdafx.h"
#include "dxException.h"
#include <cstdlib>

DxException::DxException(HRESULT hr, const std::wstring& functionName, const std::wstring& fileName, int lineNumber)
	: mError(hr), mFuncName(functionName), mFileName(fileName), mLineNumber(lineNumber)
{
}

DxException::~DxException()
{
}

const char* DxException::what() const
{
	_com_error error(mError);
	std::wstring result = mFuncName + L" failed in " + mFileName + L";  line "
		+ std::to_wstring(mLineNumber) + L";  Error : " + error.ErrorMessage();
	char msg[1024];
	int len = WideCharToMultiByte(CP_ACP, 0, result.c_str(), -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, result.c_str(), -1, msg, len, NULL, NULL);
	return msg;
}
