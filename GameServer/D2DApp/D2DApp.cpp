#include "stdafx.h"
#include "D2DApp.h"
#include "Graphics.h"
#include "Timer.h"
#include "queryWindow.h"
#include "NetClient.h"

using namespace std;

D2DApp::D2DApp()
	: Window(), mServerIPAddress()
{
	mGraphics = make_unique<GraphicScene>();
	mTimer = make_unique<Timer>();
	mNetwork = make_unique<NetClient>();
	mQueryWindow = make_unique<QueryWindow>();
}

D2DApp::~D2DApp()
{
	//CoUninitialize();
}

bool D2DApp::Init()
{
	if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED))) return false;
	if (!Window::Init({ 780, 800 }, L"D2DApp", L"MainWindow")) return false;
	if (!mGraphics->Init(mHwnd)) return false;

#ifndef STANDARD_ALONE
	if (!InitNetwork()) return false;
#endif
	return true;
}

bool D2DApp::InitNetwork()
{
	bool success = false;

#ifdef QUERY_SERVER_IP
	if(!mQueryWindow->InitWindow()) return false;
	mQueryWindow->Run();

	mServerIPAddress = mQueryWindow->GetServerIPAddress();
	success = (mNetwork->Connect(EndPoint(mServerIPAddress, SERVER_PORT)));
#else
	success = (mNetwork->Connect(EndPoint(SERVER_IP, SERVER_PORT)));
#endif

	if (success) mNetwork->Start(mGraphics.get());
	return success;
}

void D2DApp::Run()
{
	Window::ShowWindow();

	mTimer->Reset();

	MSG msg{};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			mTimer->Tick();
			
			mGraphics->Update(mTimer->ElapsedTime());
			mGraphics->Draw();
		}
	}
}

LRESULT D2DApp::OnProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_SIZE:
		mGraphics->Resize(mHwnd);
		break;

	case WM_DESTROY:
	{
	#ifndef STANDARD_ALONE
		mNetwork->Disconnect();
	#endif
		PostQuitMessage(0);
		return 0;
	}
	case WM_KEYUP:
		if (wParam == VK_ESCAPE)
		{
		#ifndef STANDARD_ALONE
			mNetwork->Disconnect();
		#endif
			PostQuitMessage(0);
			return 0;
		}
		break;

	case WM_KEYDOWN:
	{
		mNetwork->SendMovePacket(wParam);
		break;
	}
	case WM_LBUTTONDOWN:
	case WM_MOUSEMOVE:
	case WM_LBUTTONUP:
		mGraphics->OnProcessMouseInput(msg, wParam, lParam);
		break;

	default:
		return DefWindowProc(mHwnd, msg, wParam, lParam);
	}
	return 0;
}


