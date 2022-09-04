#include "stdafx.h"
#include "ChatWindow.h"
#include "NetClient.h"


#define IDC_INPUT_TEXT 0
#define IDC_EDIT_LIST 1

HWND ChatWindow::gEditList;
HWND ChatWindow::gInputText;
WNDPROC ChatWindow::gOldInputProc;
NetClient* ChatWindow::gNetPtr;


ChatWindow::ChatWindow()
	: mWinCaption{ L"Message Log" },
	  mWidth(400), mHeight(500), mHwnd(0)
{
}

ChatWindow::~ChatWindow()
{
}

bool ChatWindow::InitWindow(const std::wstring& classname)
{
	if (!RegisterWindowClass(classname)) return false;
	if (!(mHwnd = CreateWindowHandle(classname))) return false;
	return true;
}

void ChatWindow::SetNetworkPtr(NetClient* net)
{
	gNetPtr = net;
}

void ChatWindow::SetPosition(RECT& rect)
{
	SetWindowPos(mHwnd, HWND_TOP, rect.left, rect.top, 0, 0, SWP_NOSIZE);
}

bool ChatWindow::RegisterWindowClass(const std::wstring& classname)
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

HWND ChatWindow::CreateWindowHandle(const std::wstring& classname)
{
	RECT rect = { 0, 0, mWidth, mHeight};

	return CreateWindow(classname.c_str(), mWinCaption.c_str(),
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		rect.right - rect.left, rect.bottom - rect.top, NULL, NULL,
		GetModuleHandle(NULL), this);
}

void ChatWindow::AppendMessage(const std::wstring& name, const std::wstring& msg)
{
	int idx = GetWindowTextLength(gEditList);
	std::wstring chat = name + L": " + msg + L"\r\n";
	SendMessage(gEditList, EM_SETSEL, (WPARAM)idx, (LPARAM)idx);
	SendMessage(gEditList, EM_REPLACESEL, 0, (LPARAM)chat.c_str());
}

void ChatWindow::Run()
{
	::ShowWindow(mHwnd, SW_SHOW);
	::UpdateWindow(mHwnd);
}

LRESULT ChatWindow::WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
		gEditList = CreateWindow(L"EDIT", L"", 
			WS_BORDER | WS_CHILD | WS_VISIBLE | EM_SETREADONLY
			| WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL,
			10, 10, 360, 400, hwnd, (HMENU)IDC_EDIT_LIST, 0, 0);
		EnableWindow(gEditList, FALSE);

		gInputText = CreateWindow(L"EDIT", L"",
			WS_BORDER | WS_CHILD | WS_VISIBLE
			| ES_LEFT | ES_AUTOVSCROLL,
			10, 420, 360, 30, hwnd, (HMENU)IDC_INPUT_TEXT, 0, 0);
		gOldInputProc = (WNDPROC)SetWindowLongPtr(gInputText, GWLP_WNDPROC, (LONG_PTR)InputProc);
		break;

	case WM_DESTROY:		
		return 0;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

LRESULT ChatWindow::InputProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if(msg == WM_CHAR && wParam == VK_RETURN)
	{
		int size = GetWindowTextLengthW(hwnd) + 1;
		if (size <= 1) return 0;

		std::wstring message;
		message.resize(size);
		int len = GetWindowTextW(hwnd, (LPWSTR)message.c_str(), size);
			
		if (len != 0)
		{
			std::string str = WStringToString(message);
			gNetPtr->SendChatPacket(str.c_str());
			SetWindowText(hwnd, L"");				
		}
		return 0;
	}
	return CallWindowProc(gOldInputProc, hwnd, msg, wParam, lParam);
}

