#pragma once

#if defined(DEBUG) || defined(_DEBUG)
	#define _CRTDBG_MEM_ALLOC
	#include <crtdbg.h>
#endif

#include "NetCommon.h"

#include <Windows.h>
#include <wrl.h>
#include <d2d1_1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <d2d1effects.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "DWrite.lib")

#include <iostream>
#include <memory>
#include <exception>
#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <assert.h>

using Microsoft::WRL::ComPtr;
using D2D1::ColorF;


struct POINT_INT
{
	int x;
	int y;
};


inline float RandFloat()
{
	return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}


class D2DException : public std::exception
{
public:
	D2DException(const std::string& errorString);
	virtual ~D2DException() { }

	virtual const char* what() const;

private:
	std::string mErrorString;
};

