#pragma once

class DxException : public std::exception
{
public:
	DxException() = default;
	DxException(HRESULT hr, const std::wstring& functionName,
		const std::wstring& fileName, int lineNumber);
	virtual ~DxException();

	virtual const char* what() const;

	HRESULT mError = S_OK;
	std::wstring mFuncName;
	std::wstring mFileName;
	int mLineNumber = -1;
};