#pragma once

class Shape;

class GameObject
{
public:
	GameObject(ID2D1HwndRenderTarget* rt);
	virtual ~GameObject();

	void SetShape(std::unique_ptr<Shape>&& newShape);
	void SetColor(ID2D1HwndRenderTarget* rt, const D2D1_COLOR_F& color);
	void SetImage(ID2D1Bitmap* bitmap);

	void SetPosition(const D2D1_POINT_2F& pos);
	void SetWired(bool isWired) { mIsWired = isWired; }	

	D2D1_POINT_2F GetPosition() const { return mPosition; }

public:
	void Move(float dx, float dy);
	void Rotate(float degree);
	void Scale(float sx, float sy);

	void Draw(ID2D1HwndRenderTarget* rt, IDWriteTextFormat* textFormat);
	void Update(const float elapsed);
	
protected:
	void SetTransform(ID2D1HwndRenderTarget* rt);

protected:
	ComPtr<ID2D1SolidColorBrush> mSolidColorBrush;	
	std::unique_ptr<Shape> mShape;

	ID2D1Bitmap* mImage;
	
	D2D1_POINT_2F mPosition{};
	D2D1_SIZE_F mRotation{};
	D2D1_SIZE_F mScale = { 1.0f, 1.0f };

	D2D1_SIZE_F mRTSize{};

	FLOAT mRotationDegree = 0.0f;
	bool mIsWired = false;
};