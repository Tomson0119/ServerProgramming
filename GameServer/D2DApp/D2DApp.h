#pragma once

#include "Window.h"

class GraphicScene;
class Timer;
class QueryWindow;
class NetClient;

class D2DApp : public Window<D2DApp>
{
public:
	D2DApp();
	virtual ~D2DApp();

	bool Init();
	bool InitNetwork();

	void Run();

	virtual LRESULT OnProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam);

private:
	std::unique_ptr<GraphicScene> mGraphics;
	std::unique_ptr<Timer> mTimer;
	std::unique_ptr<NetClient> mNetwork;
	std::unique_ptr<QueryWindow> mQueryWindow;

	std::string mServerIPAddress;
};