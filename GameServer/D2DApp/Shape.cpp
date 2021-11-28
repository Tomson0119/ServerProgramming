#include "stdafx.h"
#include "Shape.h"

Shape::Shape()
{
}

Shape::~Shape()
{
}

Rect::Rect(const D2D1_SIZE_F& size)
	: mRectSize(size)
{
}

Rect::~Rect()
{
}

void Rect::DrawShape(
	ID2D1HwndRenderTarget* rt, 
	ID2D1SolidColorBrush* color, 
	const D2D1_POINT_2F& center)
{
	float hw = mRectSize.width * 0.5f;
	float hh = mRectSize.height * 0.5f;

	rt->DrawRectangle(
		D2D1::RectF(
			center.x - hw,
			center.y - hh,
			center.x + hw,
			center.y + hh),
		color, 2.0f);
}

void Rect::DrawShape(
	ID2D1HwndRenderTarget* rt, 
	ID2D1Bitmap* bitmap, 
	const D2D1_POINT_2F& center)
{
	float hw = mRectSize.width * 0.5f;
	float hh = mRectSize.height * 0.5f;
	
	rt->DrawBitmap(bitmap,
		D2D1::RectF(
			center.x - hw,
			center.y - hh,
			center.x + hw,
			center.y + hh));
}

void Rect::FillShape(
	ID2D1HwndRenderTarget* rt, 
	ID2D1SolidColorBrush* color, 
	const D2D1_POINT_2F& center)
{
	float hw = mRectSize.width * 0.5f;
	float hh = mRectSize.height * 0.5f;

	rt->FillRectangle(
		D2D1::RectF(
			center.x - hw,
			center.y - hh,
			center.x + hw,
			center.y + hh),
		color);
}

