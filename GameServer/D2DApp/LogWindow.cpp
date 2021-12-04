#include "stdafx.h"
#include "LogWindow.h"
#include "NetClient.h"


HWND LogWindow::gEditList;
NetClient* LogWindow::gNetPtr;

LogWindow::LogWindow()
	: mWinCaption{ L"Game Log" },
	  mWidth(600), mHeight(300), mHwnd(0)
{
}

LogWindow::~LogWindow()
{
}

bool LogWindow::InitWindow(const std::wstring& classname)
{
	if (!RegisterWindowClass(classname)) return false;
	if (!(mHwnd = CreateWindowHandle(classname))) return false;
	return true;
}

void LogWindow::SetNetworkPtr(NetClient* net)
{
	gNetPtr = net;
}

void LogWindow::SetPosition(RECT& rect)
{
	SetWindowPos(mHwnd, NULL, rect.left, rect.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
}

bool LogWindow::RegisterWindowClass(const std::wstring& classname)
{
	WNDCLASSEX wndClass{};
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WinProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = GetModuleHandle(NULL);
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = classname.c_str();
	wndClass.hIconSm = LoadIcon(wndClass.hInstance, IDI_APPLICATION);
	return (bool)RegisterClassEx(&wndClass);
}

HWND LogWindow::CreateWindowHandle(const std::wstring& classname)
{
	RECT rect = { 0, 0, mWidth, mHeight };

	return CreateWindow(classname.c_str(), mWinCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top, NULL, NULL,
		GetModuleHandle(NULL), this);
}

void LogWindow::AppendMessage(char id, const char* msg)
{
	int idx = GetWindowTextLength(gEditList);
	std::string chat = "Player " + std::to_string(id)
		+ ": " + msg + "\r\n";
	SendMessage(gEditList, EM_SETSEL, (WPARAM)idx, (LPARAM)idx);
	SendMessageA(gEditList, EM_REPLACESEL, 0, (LPARAM)chat.c_str());
}

void LogWindow::Run()
{
	::ShowWindow(mHwnd, SW_SHOW);
	::UpdateWindow(mHwnd);
}

LRESULT LogWindow::WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		gEditList = CreateWindow(L"EDIT", L"",
			WS_BORDER | WS_CHILD | WS_VISIBLE | EM_SETREADONLY
			| WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
			10, 10, 600.0f - 40.0f, 300.0f - 60.0f, hwnd, 0, 0, 0);
		EnableWindow(gEditList, FALSE);
		break;

	case WM_DESTROY:
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

