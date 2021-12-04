#pragma once

class NetClient;

class LogWindow
{
public:
	LogWindow();
	virtual ~LogWindow();

	bool InitWindow(const std::wstring& classname);
	void SetNetworkPtr(NetClient* net);
	void SetPosition(RECT& rect);
	void Run();

	static void AppendMessage(char id, const char* msg);

private:
	bool RegisterWindowClass(const std::wstring& classname);
	HWND CreateWindowHandle(const std::wstring& classname);

public:
	static LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	std::wstring mWinCaption;

	HWND mHwnd;
	static HWND gEditList;

	int mWidth;
	int mHeight;

	static NetClient* gNetPtr;
};