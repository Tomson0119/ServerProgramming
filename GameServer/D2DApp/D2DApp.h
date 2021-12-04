#pragma once

#include "Window.h"

class GraphicScene;
class Timer;
class QueryWindow;
class NetClient;
class ChatWindow;
class LogWindow;

class D2DApp : public Window<D2DApp>
{
public:
	D2DApp();
	virtual ~D2DApp();

	bool Init();
	bool InitNetwork();

	void RepositionWindows();
	void Run();

	virtual LRESULT OnProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam);

private:
	std::unique_ptr<GraphicScene> mGraphics;
	std::unique_ptr<Timer> mTimer;
	std::unique_ptr<NetClient> mNetwork;

	std::unique_ptr<QueryWindow> mQueryWindow;
	std::unique_ptr<ChatWindow> mChatWindow;
	std::unique_ptr<LogWindow> mLogWindow;

	std::string mServerIPAddress;
};