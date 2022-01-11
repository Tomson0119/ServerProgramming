#pragma once

class GameObject;
class BitmapLoader;

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

	void InitializePlayer(sc_packet_login_ok& player_info, const char* name);
	void CreateNewObject(int id, char obj_type, const char* name, short x, short y);
	void UpdatePlayerPosition(int id, short x, short y);
	void UpdatePlayerChat(int id, char* msg);
	void UpdatePlayerStatus(sc_packet_status_change& status);
	void EraseObject(int id);
	
	bool FindMovingObjectID(int id);

	void CreateAttackArea();
	void EraseTimeOutObjects();

	void Quit();

public:
	std::wstring GetPlayerName(int id);

private:
	bool CreateD2D1Resources(const HWND& hwnd);
	bool CreateDirectWriteResources();

public:	
	void OnProcessKeyInput(const float elapsed) { }
	void OnProcessMouseInput(UINT msg, WPARAM wParam, LPARAM lParam) { }
	
private:
	ComPtr<ID2D1Factory> mFactory;
	ComPtr<ID2D1HwndRenderTarget> mRenderTarget;
	ComPtr<ID2D1Layer> mLayer;

	ComPtr<IDWriteFactory> mWriteFactory;
	ComPtr<IDWriteTextFormat> mTextFormat;
	ComPtr<ID2D1SolidColorBrush> mLabelColorBrush;

	std::vector<std::unique_ptr<BitmapLoader>> mBitmaps;

	D2D1_POINT_2F mPlayerOffset;

	D2D1_MATRIX_3X2_F mCameraMatrix;
	D2D1_POINT_2F mCameraPosition;

	int mPlayerID;
	std::mutex mMovingObjectsLock;
	std::unordered_map<int, std::unique_ptr<GameObject>> mMovingObjects;
	std::vector<std::unique_ptr<GameObject>> mStaticObjects;
	std::unordered_set<std::unique_ptr<GameObject>> mEffects;

	float mChatShowedTime;

	HWND mAppHwnd;
};