#include "stdafx.h"
#include "GameObject.h"
#include "Shape.h"


GameObject::GameObject(ID2D1HwndRenderTarget* rt)
	: mRTSize(rt->GetSize()), mChatActive(false), mPosX(0), mPosY(0)
{
}

GameObject::~GameObject()
{
}

void GameObject::SetColor(ID2D1HwndRenderTarget* rt, const D2D1_COLOR_F& color)
{
	ThrowIfFailed(rt->CreateSolidColorBrush(color, mSolidColorBrush.GetAddressOf()));
}

void GameObject::SetChat(const std::wstring& chat)
{
	mChat = chat;
	mChatActive = true;
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
	
	if(mImage)
		mShape->DrawShape(rt, mImage);
	else
	{
		if (mIsWired)
			mShape->DrawShape(rt, mSolidColorBrush.Get());
		else
			mShape->FillShape(rt, mSolidColorBrush.Get());
	}
}

void GameObject::DrawIDLabel(
	ID2D1HwndRenderTarget* rt, 
	IDWriteTextFormat* textFormat,
	ID2D1SolidColorBrush* textColor)
{
	if (mID.size() == 0)
		return;

	D2D1_SIZE_F shape_size = mShape->GetSize();
	float hh = shape_size.height * 0.5f;
	float w = shape_size.width * 1.5f;

	float font_size = textFormat->GetFontSize();
	rt->DrawText(mID.c_str(), (UINT32)mID.size(), textFormat, { -w, hh, +w, hh + font_size }, textColor);
}

void GameObject::DrawChatLabel(
	ID2D1HwndRenderTarget* rt,
	IDWriteTextFormat* textFormat,
	ID2D1SolidColorBrush* textColor)
{
	if (mChat.size() == 0)
		return;

	D2D1_SIZE_F shape_size = mShape->GetSize();
	float hh = shape_size.height * 0.5f;
	float w = shape_size.width * 1.5f;
	float offset_y = 10.0f;

	float font_size = textFormat->GetFontSize();
	rt->DrawText(mChat.c_str(), (UINT32)mChat.size(), textFormat, { -w, -hh - font_size- offset_y, +w, -hh }, textColor);
}

void GameObject::DrawPositionLabel(ID2D1HwndRenderTarget* rt, IDWriteTextFormat* textFormat, ID2D1SolidColorBrush* textColor)
{
	SetTransform(rt);

	D2D1_SIZE_F rtSize = rt->GetSize();
	float left = -rtSize.width * 0.5f;
	float top = -rtSize.height * 0.5f;

	std::wstring label = L"( " + std::to_wstring(mPosX) + L", " + std::to_wstring(mPosY) + L" )";
	rt->DrawText(label.c_str(), (UINT32)label.size(), textFormat, 
		{ left, top, left + 100.0f, top + 50.0f }, textColor);
}

void GameObject::SetTransform(ID2D1HwndRenderTarget* rt)
{
	D2D1_MATRIX_3X2_F translation = D2D1::Matrix3x2F::Translation(mPosition.x, mPosition.y);
	D2D1_MATRIX_3X2_F rotation = D2D1::Matrix3x2F::Rotation(mRotationDegree, mPosition);
	D2D1_MATRIX_3X2_F scale = D2D1::Matrix3x2F::Scale(mScale, mPosition);

	rt->SetTransform(*mCameraMat * translation);
}

void GameObject::Update(const float elapsed)
{
	if (mChatActive)
	{
		static float accum_time = 0.0f;
		accum_time += elapsed;
		if (accum_time > 1.0f)
		{
			mChat.clear();
			accum_time = 0.0f;
			mChatActive = false;
		}
	}
}