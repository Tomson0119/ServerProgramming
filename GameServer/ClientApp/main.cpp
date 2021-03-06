#include "stdafx.h"
#include "gameFramework.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR, int mCmdShow)
{
#if defined(DEBUG) || defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	if (AllocConsole() == TRUE)
	{
		FILE* file;
		freopen_s(&file, "CONOUT$", "rb", stdin);
		freopen_s(&file, "CONOUT$", "wb", stdout);
		freopen_s(&file, "CONOUT$", "wb", stderr);
		std::ios::sync_with_stdio();
	}
	
#endif
	try
	{
		GameFramework app;
		app.SetResolution(800, 800);

		if (!app.InitFramework())
			return 0;		
		app.Run();
	}
	catch (std::exception& ex)
	{
		MessageBoxA(NULL, ex.what(), "ERROR", MB_OK);
		return 0;
	}
}