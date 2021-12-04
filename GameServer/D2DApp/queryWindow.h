#pragma once

#include "Window.h"

class QueryWindow : public Window<QueryWindow>
{
public:
	QueryWindow();
	virtual ~QueryWindow();

	bool InitWindow(const std::wstring& classname);
	void Run();

	void SetLabel(const std::wstring& label) { mLabel = label; }
	std::string GetAnswer() const { return mAnswer; }

public:
	virtual LRESULT OnProcessMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	std::wstring mWinCaption;

	int mWidth;
	int mHeight;
	
	HWND mTextBox{};

	std::wstring mLabel;
	std::string mAnswer = "";
};