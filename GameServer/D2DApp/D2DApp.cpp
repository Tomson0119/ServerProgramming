#include "stdafx.h"
#include "D2DApp.h"
#include "Graphics.h"
#include "Timer.h"
#include "queryWindow.h"
#include "NetClient.h"
#include "ChatWindow.h"
#include "LogWindow.h"

using namespace std;

D2DApp::D2DApp()
	: Window(), mServerIPAddress()
{
	mGraphics = make_unique<GraphicScene>();
	mTimer = make_unique<Timer>();
	mNetwork = make_unique<NetClient>();
	mQueryWindow = make_unique<QueryWindow>();
	mChatWindow = make_unique<ChatWindow>();
	mLogWindow = make_unique<LogWindow>();
}

D2DApp::~D2DApp()
{
	//CoUninitialize();
}

bool D2DApp::Init()
{
	if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED))) return false;
	if (!Window::Init({ 790, 810 }, L"D2DApp", L"MainWindow")) return false;	
	if (!mChatWindow->InitWindow(L"ChatWindow")) return false;
	if (!mLogWindow->InitWindow(L"LogWindow")) return false;
	if (!mGraphics->Init(mHwnd)) return false;
	if (!InitNetwork()) return false;
	return true;
}

bool D2DApp::InitNetwork()
{
	bool success = false;

#ifndef LOCAL_ADDRESS
	if(!mQueryWindow->InitWindow(L"IPQueryWindow")) return false;
	mQueryWindow->SetLabel(L"Server IP Address");
	mQueryWindow->Run();

	mServerIPAddress = mQueryWindow->GetAnswer();
	success = (mNetwork->Connect(EndPoint(mServerIPAddress, SERVER_PORT)));
#else
	success = (mNetwork->Connect(EndPoint("127.0.0.1", SERVER_PORT)));
#endif
	if (success == false) return success;

	std::string user_name = "pepsi90";
#ifdef QUERY_ID
	if (!mQueryWindow->InitWindow(L"IDQueryWindow")) return false;
	mQueryWindow->SetLabel(L"Enter user name");
	mQueryWindow->Run();

	user_name = mQueryWindow->GetAnswer();
#endif
	if (success)
	{
		mChatWindow->SetNetworkPtr(mNetwork.get());
		mNetwork->SetInterfaces(mGraphics.get(), mChatWindow.get(), mLogWindow.get());
		mNetwork->Start(user_name.c_str());
	}
	return success;
}

void D2DApp::RepositionWindows()
{
	RECT rect{};
	GetWindowRect(mHwnd, &rect);
	rect.left += mWindowWidth;
	mChatWindow->SetPosition(rect);
	rect.top += mWindowHeight - 300.0f;
	mLogWindow->SetPosition(rect);
	
}

void D2DApp::Run()
{
	Window::ShowWindow();
	RepositionWindows();
	mChatWindow->Run();
	mLogWindow->Run();

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
		mNetwork->Disconnect();
		PostQuitMessage(0);
		return 0;
	}

	case WM_MOVING:
	{
		RepositionWindows();
		break;
	}
	case WM_KEYUP:
		if (wParam == VK_ESCAPE)
		{
			mNetwork->Disconnect();
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


