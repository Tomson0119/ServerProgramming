#pragma once

#include "Window.h"

class QueryWindow : public Window<QueryWindow>
{
public:
	QueryWindow();
	virtual ~QueryWindow();

	bool InitWindow();
	void Run();

	std::string GetServerIPAddress() const { return mServerIPAddress; }

public:
	virtual PCWSTR ClassName() const { return L"Query Window"; }
	virtual LRESULT OnProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	const std::wstring mWinCaption = L"QueryWindow";

	const int mWidth = 600;
	const int mHeight = 200;

	HWND mTextBox{};

	std::string mServerIPAddress = "";
};