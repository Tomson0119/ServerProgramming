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
	ID2D1SolidColorBrush* color)
{
	float hw = mRectSize.width * 0.5f;
	float hh = mRectSize.height * 0.5f;

	rt->DrawRectangle(D2D1::RectF(-hw, -hh, +hw, +hh), color, 2.0f);
}

void Rect::DrawShape(
	ID2D1HwndRenderTarget* rt, 
	ID2D1Bitmap* bitmap)
{
	float hw = mRectSize.width * 0.5f;
	float hh = mRectSize.height * 0.5f;
	
	rt->DrawBitmap(bitmap, D2D1::RectF(-hw, -hh, +hw, +hh));
}

void Rect::FillShape(
	ID2D1HwndRenderTarget* rt, 
	ID2D1SolidColorBrush* color)
{
	float hw = mRectSize.width * 0.5f;
	float hh = mRectSize.height * 0.5f;

	rt->FillRectangle(D2D1::RectF(-hw, -hh, +hw, + hh),	color);
}


Circle::Circle(float radius)
	: mRadius(radius)
{
}

Circle::~Circle()
{
}

void Circle::DrawShape(
	ID2D1HwndRenderTarget* rt, 
	ID2D1SolidColorBrush* color)
{
	rt->DrawEllipse(D2D1::Ellipse({ 0.0f, 0.0f }, mRadius, mRadius), color, 2.0f);
}

void Circle::FillShape(
	ID2D1HwndRenderTarget* rt,
	ID2D1SolidColorBrush* color)
{
	rt->FillEllipse(D2D1::Ellipse({ 0.0f, 0.0f }, mRadius, mRadius), color);
}
