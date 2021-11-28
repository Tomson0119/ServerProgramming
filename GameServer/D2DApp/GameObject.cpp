#include "stdafx.h"
#include "GameObject.h"
#include "Shape.h"


GameObject::GameObject(ID2D1HwndRenderTarget* rt)
	: mRTSize(rt->GetSize())
{
	
}

GameObject::~GameObject()
{
}

void GameObject::SetShape(std::unique_ptr<Shape>&& newShape)
{
	mShape = std::move(newShape);
}

void GameObject::SetColor(ID2D1HwndRenderTarget* rt, const D2D1_COLOR_F& color)
{
	HRESULT result = rt->CreateSolidColorBrush(color, mSolidColorBrush.GetAddressOf());
	if (FAILED(result)) throw D2DException("Failed to create brush");
}

void GameObject::SetImage(ID2D1Bitmap* bitmap)
{
	mImage = bitmap;
}

void GameObject::SetPosition(const D2D1_POINT_2F& pos)
{
	mPosition.x = mRTSize.width * 0.5f + pos.x;
	mPosition.y = mRTSize.height * 0.5f + pos.y;
}

void GameObject::Move(float dx, float dy)
{
	mPosition.x += dx;
	mPosition.y -= dy;
}

void GameObject::Rotate(float degree)
{
	mRotationDegree += degree;
}

void GameObject::Scale(float sx, float sy)
{
	mScale.width = sx;
	mScale.height = sy;
}

void GameObject::Draw(ID2D1HwndRenderTarget* rt, IDWriteTextFormat* textFormat)
{
	SetTransform(rt);

	
	mShape->DrawShape(rt, mImage, mPosition);
		
	/*if (mIsWired)
		mShape->DrawShape(rt, mSolidColorBrush.Get(), mCenterCoord);
	else
		mShape->FillShape(rt, mSolidColorBrush.Get(), mCenterCoord);

	std::wstring str = L"안녕하신가요?";
	rt->DrawText(str.c_str(), str.size(), textFormat,
		D2D1::RectF(0, 0, rt->GetSize().width, rt->GetSize().height),
		mSolidColorBrush.Get());*/
}

void GameObject::SetTransform(ID2D1HwndRenderTarget* rt)
{
	D2D1_MATRIX_3X2_F rotation = D2D1::Matrix3x2F::Rotation(mRotationDegree, mPosition);
	D2D1_MATRIX_3X2_F scale = D2D1::Matrix3x2F::Scale(mScale, mPosition);

	rt->SetTransform(scale * rotation);
}

void GameObject::Update(const float elapsed)
{
	//mRotationDegree += 100.0f * elapsed;
}