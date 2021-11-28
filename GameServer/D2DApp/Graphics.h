#pragma once

class GameObject;
class BitmapLoader;

enum Layer
{
	STATIC,
	PLAYERS,
};

class GraphicScene
{
public:
	GraphicScene();
	~GraphicScene();

	bool Init(const HWND& hwnd);

	void Draw();	
	void Update(const float elapsed);
	void Resize(const HWND& hwnd);

	void BuildMap();
	void BuildImages();

private:
	bool CreateD2D1Resources(const HWND& hwnd);
	bool CreateDirectWriteResources();

public:	
	void OnProcessKeyInput(const float elapsed);
	void OnProcessMouseInput(UINT msg, WPARAM wParam, LPARAM lParam);
	
private:
	ComPtr<ID2D1Factory> mFactory;
	ComPtr<ID2D1HwndRenderTarget> mRenderTarget;
	ComPtr<ID2D1Layer> mLayer;

	ComPtr<IDWriteFactory> mWriteFactory;
	ComPtr<IDWriteTextFormat> mTextFormat;

	ComPtr<ID2D1DeviceContext> mDeviceContext;
	ComPtr<ID2D1Effect> mTileEffect;
	std::vector<std::unique_ptr<BitmapLoader>> mBitmaps;

	GameObject* mPlayer = nullptr;

	std::array<std::vector<std::unique_ptr<GameObject>>, 2> mGameObjects;
};