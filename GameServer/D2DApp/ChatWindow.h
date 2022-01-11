#pragma once

class NetClient;

class ChatWindow
{
public:
	ChatWindow();
	virtual ~ChatWindow();

	bool InitWindow(const std::wstring& classname);
	void SetNetworkPtr(NetClient* net);
	void SetPosition(RECT& rect);
	void Run();

	static void AppendMessage(const std::wstring& name, const std::wstring& msg);

private:
	bool RegisterWindowClass(const std::wstring& classname);
	HWND CreateWindowHandle(const std::wstring& classname);

public:
	static LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK InputProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	std::wstring mWinCaption;

	HWND mHwnd;
	static HWND gEditList;
	static HWND gInputText;

	static WNDPROC gOldInputProc;

	int mWidth;
	int mHeight;

	static NetClient* gNetPtr;
};