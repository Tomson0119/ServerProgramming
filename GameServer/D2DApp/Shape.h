#pragma once

class Shape
{
public:
	Shape();
	virtual ~Shape();

	virtual void DrawShape(
		ID2D1HwndRenderTarget* rt,
		ID2D1SolidColorBrush* color) = 0;

	virtual void DrawShape(
		ID2D1HwndRenderTarget* rt,
		ID2D1Bitmap* bitmap) = 0;

	virtual void FillShape(
		ID2D1HwndRenderTarget* rt, 
		ID2D1SolidColorBrush* color) = 0;

	virtual D2D1_SIZE_F GetSize() const = 0;
};

class Rect : public Shape
{
public:
	Rect(const D2D1_SIZE_F& size);
	virtual ~Rect();

public:
	virtual void DrawShape(
		ID2D1HwndRenderTarget* rt, 
		ID2D1SolidColorBrush* color) override;

	virtual void DrawShape(
		ID2D1HwndRenderTarget* rt,
		ID2D1Bitmap* bitmap) override;

	virtual void FillShape(
		ID2D1HwndRenderTarget* rt, 
		ID2D1SolidColorBrush* color) override;

	virtual D2D1_SIZE_F GetSize() const { return mRectSize; }

private:
	D2D1_SIZE_F mRectSize;
};

class Circle : public Shape
{
public:
	Circle(float radius);
	virtual ~Circle();

public:
	virtual void DrawShape(
		ID2D1HwndRenderTarget* rt,
		ID2D1SolidColorBrush* color) override;

	virtual void DrawShape(
		ID2D1HwndRenderTarget* rt,
		ID2D1Bitmap* bitmap) override { }

	virtual void FillShape(
		ID2D1HwndRenderTarget* rt,
		ID2D1SolidColorBrush* color) override;

	virtual D2D1_SIZE_F GetSize() const { return { mRadius * 2, mRadius * 2 }; }

private:
	float mRadius = 0.0f;
};

class Triangle : public Shape
{

};