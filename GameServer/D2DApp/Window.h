#pragma once

template<class T>
class Window
{
public:
	Window() { }
	virtual ~Window() { }

	bool Init(
		const POINT_INT& res, 
		const std::wstring& winname,
		const std::wstring& classname)
	{
		mWindowWidth = res.x;
		mWindowHeight = res.y;
		mWindowName = winname.c_str();
		mClassName = classname.c_str();

		if (!RegisterWindowClass())
			return false;

		if (!(mHwnd = CreateWindowHandle()))
			return false;

		return true;
	}

	void ShowWindow()
	{
		if (mHwnd)
		{
			::ShowWindow(mHwnd, SW_SHOW);
			::UpdateWindow(mHwnd);
		}
	}

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		T* pThis = nullptr;

		if (msg == WM_CREATE)
		{
			CREATESTRUCT* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
			pThis = reinterpret_cast<T*>(cs->lpCreateParams);
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
			pThis->mHwnd = hwnd;
		}
		else
		{
			pThis = reinterpret_cast<T*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
		}

		if (pThis)
			return pThis->OnProcessMessage(msg, wParam, lParam);
		else
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}

private:
	bool RegisterWindowClass()
	{
		WNDCLASSEX wndClass{};
		wndClass.cbSize = sizeof(WNDCLASSEX);
		wndClass.style = CS_HREDRAW | CS_VREDRAW;
		wndClass.lpfnWndProc = T::WindowProc;
		wndClass.cbClsExtra = 0;
		wndClass.cbWndExtra = 0;
		wndClass.hInstance = GetModuleHandle(NULL);
		wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wndClass.lpszMenuName = NULL;
		wndClass.lpszClassName = mClassName;
		wndClass.hIconSm = LoadIcon(wndClass.hInstance, IDI_APPLICATION);
		return (bool)RegisterClassEx(&wndClass);
	}

	HWND CreateWindowHandle()
	{
		RECT rect = { 0, 0, mWindowWidth, mWindowHeight };

		return CreateWindow(mClassName, mWindowName,
			WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
			rect.right - rect.left, rect.bottom - rect.top, NULL, NULL,
			GetModuleHandle(NULL), this);
	}

protected:
	virtual LRESULT OnProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam) = 0;

protected:
	HWND mHwnd = NULL;

	LPCWSTR mWindowName = NULL;
	LPCWSTR mClassName = NULL;
	
	int mWindowWidth  = 0;
	int mWindowHeight = 0;
};