#include "stdafx.h"
#include "D2DApp.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int nCmdShow)
{
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		D2DApp app;
		if (!app.Init())
			throw D2DException(L"Failed to initialize");
		app.Run();

		return 0;
	}
	catch (D2DException& ex)
	{
		MessageBoxW(NULL, ex.what(), L"Error", MB_OK);
		return 0;
	}
}