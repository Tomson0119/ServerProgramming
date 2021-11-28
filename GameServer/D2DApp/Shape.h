#pragma once

class Shape
{
public:
	Shape();
	virtual ~Shape();

	virtual void DrawShape(
		ID2D1HwndRenderTarget* rt,
		ID2D1SolidColorBrush* color,
		const D2D1_POINT_2F& center) = 0;

	virtual void DrawShape(
		ID2D1HwndRenderTarget* rt,
		ID2D1Bitmap* bitmap,
		const D2D1_POINT_2F& center) = 0;

	virtual void FillShape(
		ID2D1HwndRenderTarget* rt, 
		ID2D1SolidColorBrush* color,
		const D2D1_POINT_2F& center) = 0;
};

class Rect : public Shape
{
public:
	Rect(const D2D1_SIZE_F& size);
	virtual ~Rect();

public:
	virtual void DrawShape(
		ID2D1HwndRenderTarget* rt, 
		ID2D1SolidColorBrush* color, 
		const D2D1_POINT_2F& center) override;

	virtual void DrawShape(
		ID2D1HwndRenderTarget* rt,
		ID2D1Bitmap* bitmap,
		const D2D1_POINT_2F& center) override;

	virtual void FillShape(
		ID2D1HwndRenderTarget* rt, 
		ID2D1SolidColorBrush* color, 
		const D2D1_POINT_2F& center) override;

private:
	D2D1_SIZE_F mRectSize;
};

class Circle : public Shape
{

};

class Triangle : public Shape
{

};