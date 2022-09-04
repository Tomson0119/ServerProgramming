#pragma once

#if defined(DEBUG) || defined(_DEBUG)
	#define _CRTDBG_MEM_ALLOC
	#include <crtdbg.h>
#endif

#include "NetCommon.h"

#include <Windows.h>
#include <wrl.h>
#include <d3d11_3.h>
#include <d2d1_3.h>
#include <d2d1effects_2.h>
#include <dwrite_3.h>
#include <d2d1helper.h>
#include <wincodec.h>
#include <comdef.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "DWrite.lib")
#pragma comment(lib, "d3d11.lib")

#include <iostream>
#include <memory>
#include <exception>
#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <assert.h>
#include <algorithm>


using Microsoft::WRL::ComPtr;
using D2D1::ColorF;

#define LOCAL_ADDRESS
#define QUERY_ID

struct POINT_INT
{
	int x;
	int y;
};

inline std::wstring StringToWString(const std::string& str)
{
	int size = MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstr(size, 0);
	MultiByteToWideChar(CP_ACP, 0, &str[0], (int)str.size(), &wstr[0], size);
	return wstr;
}

inline std::wstring CharToWString(const char* str)
{
	std::string s_str(str);
	int size = MultiByteToWideChar(CP_ACP, 0, &s_str[0], (int)s_str.size(), NULL, 0);
	std::wstring wstr(size, 0);
	MultiByteToWideChar(CP_ACP, 0, &s_str[0], (int)s_str.size(), &wstr[0], size);
	return wstr;
}

inline std::string WStringToString(const std::wstring& wstr)
{
	int size = WideCharToMultiByte(CP_ACP, 0, &wstr[0], -1, NULL, 0, NULL, NULL);
	std::string retStr(size, 0);
	WideCharToMultiByte(CP_ACP, 0, &wstr[0], -1, &retStr[0], size, NULL, NULL);
	return retStr;
}


inline float RandFloat()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}


inline std::wstring AnsiToWString(const std::string& str)
{
	WCHAR buffer[512];
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, buffer, 512);
	return std::wstring(buffer);
}

#ifndef ThrowIfFailed
#define ThrowIfFailed(f)												\
{																		\
	HRESULT hr_ = (f);													\
	std::wstring wfn = AnsiToWString(__FILE__);							\
	if (FAILED(hr_))	{ throw D2DException(hr_, L#f, wfn, __LINE__); } \
}																		
#endif


class D2DException
{
public:
	D2DException(const std::wstring& info);
	D2DException(HRESULT hr, const std::wstring& functionName,
		const std::wstring& fileName, int lineNumber);
	~D2DException();

	const wchar_t* what() const;

private:
	std::wstring mErrorString;

	HRESULT mError = S_OK;
	std::wstring mFuncName;
	std::wstring mFileName;
	int mLineNumber = -1;
};