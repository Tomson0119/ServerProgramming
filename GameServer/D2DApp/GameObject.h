#pragma once

class Shape;

class GameObject
{
public:
	GameObject(ID2D1HwndRenderTarget* rt);
	virtual ~GameObject();

	void SetColor(ID2D1HwndRenderTarget* rt, const D2D1_COLOR_F& color);
	void SetShape(std::unique_ptr<Shape>&& newShape) { mShape = std::move(newShape); }
	void SetImage(ID2D1Bitmap* bitmap) { mImage = bitmap; }
	void SetCamera(D2D1_MATRIX_3X2_F* cameraMat) { mCameraMat = cameraMat; }

	void SetID(const std::wstring& id) { mID = id; }
	void SetWired(bool isWired) { mIsWired = isWired; }
	void SetCoord(short x, short y) { mPosX = x, mPosY = y; }
	void SetChat(const std::wstring& chat);
	void SetPosition(const D2D1_POINT_2F& pos);

	D2D1_POINT_2F GetPosition() const { return mPosition; }
	std::pair<short, short> GetCoord() const { return { mPosX, mPosY }; }

public:
	void Move(float dx, float dy);
	void Rotate(float degree);
	void Scale(float sx, float sy);

	virtual void Draw(
		ID2D1HwndRenderTarget* rt,
		IDWriteTextFormat* textFormat,
		ID2D1SolidColorBrush* textColor);
	void DrawIDLabel(
		ID2D1HwndRenderTarget* rt, 
		IDWriteTextFormat* textFormat,
		ID2D1SolidColorBrush* textColor);
	void DrawChatLabel(
		ID2D1HwndRenderTarget* rt, 
		IDWriteTextFormat* textFormat,
		ID2D1SolidColorBrush* textColor);

	void Update(const float elapsed);
	
protected:
	void SetTransform(ID2D1HwndRenderTarget* rt);

protected:
	ComPtr<ID2D1SolidColorBrush> mSolidColorBrush;
	ComPtr<IDWriteTextLayout> mTextLayout;

	std::unique_ptr<Shape> mShape;

	ID2D1Bitmap* mImage;
	
	D2D1_POINT_2F mPosition{};
	D2D1_SIZE_F mRotation{};
	D2D1_SIZE_F mScale = { 1.0f, 1.0f };

	D2D1_SIZE_F mRTSize{};
	D2D1_MATRIX_3X2_F* mCameraMat;

	std::wstring mID;
	std::wstring mChat;

	short mPosX, mPosY;
	bool mChatActive;
	float mChatShowedTime;

	FLOAT mRotationDegree = 0.0f;
	bool mIsWired = false;
};

class Player : public GameObject
{
public:
	Player(ID2D1HwndRenderTarget* rt);
	virtual ~Player();

	void SetInfo(short level, short hp, short max_hp, short exp);

	virtual void Draw(
		ID2D1HwndRenderTarget* rt, 
		IDWriteTextFormat* textFormat,
		ID2D1SolidColorBrush* textColor);

private:
	void DrawPositionLabel(
		ID2D1HwndRenderTarget* rt,
		IDWriteTextFormat* textFormat,
		ID2D1SolidColorBrush* textColor);

	void DrawPlayerInfoLabel(
		ID2D1HwndRenderTarget* rt,
		IDWriteTextFormat* textFormat,
		ID2D1SolidColorBrush* textColor);

private:
	short mHP;
	short mMaxHP;
	short mEXP;
	short mLevel;
};