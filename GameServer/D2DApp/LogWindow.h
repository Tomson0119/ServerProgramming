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

	static void AppendLog(const std::wstring& op_name, int value, char log_type);

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