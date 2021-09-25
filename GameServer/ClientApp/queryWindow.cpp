#include "stdafx.h"
#include "queryWindow.h"

QueryWindow::QueryWindow()
{
}

QueryWindow::~QueryWindow()
{
}

bool QueryWindow::InitWindow()
{
	if (!BaseWin::InitWindow(mWinCaption.c_str(), mWidth, mHeight))
		return false;
	return true;
}

void QueryWindow::Run()
{
	MSG msg{};

	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}

LRESULT QueryWindow::OnProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	std::wstring text = L"Server IP Address";
	PAINTSTRUCT ps{};
	HDC hdc{};
	HFONT font{}, oldfont{};

	switch (uMsg)
	{
	case WM_CREATE:
		mTextBox = CreateWindow(L"EDIT", L"", WS_BORDER | WS_CHILD | WS_VISIBLE,
			60, 120, 400, 30, m_hwnd, 0, 0, 0);
		CreateWindow(L"BUTTON", L"Done", WS_VISIBLE | WS_CHILD | WS_BORDER,
			480, 120, 70, 30, m_hwnd, (HMENU)1, 0, 0);
		break;
	case WM_PAINT:	
		hdc = BeginPaint(m_hwnd, &ps);
		font = CreateFont(30, 0, 0, 0, FW_BOLD, 0, 0, 0, ANSI_CHARSET, 0, 0, 0, 0, NULL);
		oldfont = (HFONT)SelectObject(hdc, font);
		TextOut(hdc, mWidth/2 - 100, mHeight/3, text.c_str(), text.size());
		EndPaint(m_hwnd, &ps);
		break;

	case WM_COMMAND:
		if (LOWORD(wParam) == 1)
		{
			mServerIPAddress.resize(GetWindowTextLength(mTextBox) + 1);
			int len = GetWindowText(mTextBox, (LPWSTR)mServerIPAddress.c_str(), mServerIPAddress.size());
			if (len != 0)
			{
				PostMessage(m_hwnd, WM_CLOSE, 0, 0);
			}
		}
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	default:
		return DefWindowProc(m_hwnd, uMsg, wParam, lParam);
	}
	return 0;
}
