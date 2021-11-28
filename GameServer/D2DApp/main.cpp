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
			throw D2DException("Failed to initialize");
		app.Run();

		return 0;
	}
	catch (std::exception& ex)
	{
		MessageBoxA(NULL, ex.what(), "Error", MB_OK);
		return 0;
	}
}