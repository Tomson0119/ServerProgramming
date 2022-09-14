#include "stdafx.h"
#include "GameObject.h"
#include "Shape.h"


GameObject::GameObject(ID2D1HwndRenderTarget* rt)
	: mRTSize(rt->GetSize()), mChatActive(false),
	  mPosX(0), mPosY(0), mChatShowedTime(0.0f),
	  mDurationTime(0)
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
	mChatShowedTime = 0.0f;
}

void GameObject::SetPosition(const D2D1_POINT_2F& pos)
{
	mPosition.x = mRTSize.width * 0.5f + pos.x;
	mPosition.y = mRTSize.height * 0.5f + pos.y;
}

void GameObject::SetDurationTime(std::chrono::milliseconds time)
{
	mDurationTime = time;
	mStartTime = std::chrono::system_clock::now();
}

bool GameObject::IsTimeOut()
{
	if (mDurationTime == std::chrono::milliseconds(0)) return false;
	return (mStartTime + mDurationTime < std::chrono::system_clock::now());
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

void GameObject::Draw(
	ID2D1HwndRenderTarget* rt, 
	IDWriteTextFormat* textFormat,
	ID2D1SolidColorBrush* textColor)
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

	DrawIDLabel(rt, textFormat, textColor);
	DrawChatLabel(rt, textFormat, textColor);
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
	if (mChat.size() == 0 || mShape.get() == nullptr)
		return;

	D2D1_SIZE_F shape_size = mShape->GetSize();
	float hh = shape_size.height * 0.5f;
	float w = shape_size.width * 1.5f;
	float offset_y = 10.0f;

	float font_size = textFormat->GetFontSize();
	rt->DrawText(mChat.c_str(), (UINT32)mChat.size(), textFormat, { -w, -hh - font_size- offset_y, +w, -hh }, textColor);
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
		mChatShowedTime += elapsed;
		if (mChatShowedTime > 1.0f)
		{
			mChat.clear();
			mChatShowedTime = 0.0f;
			mChatActive = false;
		}
	}
}

Player::Player(ID2D1HwndRenderTarget* rt)
	: GameObject(rt), mHP(0), mMaxHP(0), mEXP(0), mLevel(0)
{
}

Player::~Player()
{
}

void Player::SetInfo(short level, short hp, short max_hp, short exp)
{
	mLevel = level;
	mHP = hp;
	mMaxHP = max_hp;
	mEXP = exp;
}

void Player::Draw(
	ID2D1HwndRenderTarget* rt, 
	IDWriteTextFormat* textFormat,
	ID2D1SolidColorBrush* textColor)
{
	GameObject::Draw(rt, textFormat, textColor);
	DrawPositionLabel(rt, textFormat, textColor);
	DrawPlayerInfoLabel(rt, textFormat, textColor);
}

void Player::DrawPositionLabel(
	ID2D1HwndRenderTarget* rt, 
	IDWriteTextFormat* textFormat,
	ID2D1SolidColorBrush* textColor)
{
	D2D1_SIZE_F rtSize = rt->GetSize();
	float hw = rtSize.width * 0.5f;
	float hh = rtSize.height * 0.5f;

	std::wstring label = L"( " + std::to_wstring(mPosX) + L", " + std::to_wstring(mPosY) + L" )";
	rt->DrawText(label.c_str(), (UINT32)label.size(), textFormat,
		{ -hw * 0.5f, -hh + 10.0f, hw * 0.5f, -hh + 30.0f }, textColor);
}

void Player::DrawPlayerInfoLabel(
	ID2D1HwndRenderTarget* rt,
	IDWriteTextFormat* textFormat,
	ID2D1SolidColorBrush* textColor)
{
	D2D1_SIZE_F rtSize = rt->GetSize();
	float hw = rtSize.width * 0.5f;
	float hh = rtSize.height * 0.5f;

	std::wstring label = 
		L"LEVEL. " + std::to_wstring(mLevel) + L"\t\tHP. " + std::to_wstring(mHP) 
		+ L" / " + std::to_wstring(mMaxHP) + L"\t\tEXP. " + std::to_wstring(mEXP)
		+ L" / " + std::to_wstring(100 * (int)pow(2, mLevel-1));
	rt->DrawText(label.c_str(), (UINT32)label.size(), textFormat,
		{ -hw, hh - 30.0f, hw, hh - 10.0f }, textColor);
}
