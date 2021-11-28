#include "stdafx.h"
#include "Graphics.h"
#include "GameObject.h"
#include "BitmapLoader.h"
#include "Shape.h"

using namespace std;


GraphicScene::GraphicScene()
{
	mBitmaps.push_back(std::make_unique<BitmapLoader>());
}

GraphicScene::~GraphicScene()
{
}

bool GraphicScene::Init(const HWND& hwnd)
{
	if (!CreateD2D1Resources(hwnd))
		return false;

	if (!CreateDirectWriteResources())
		return false;

	BuildImages();
	return true;
}

bool GraphicScene::CreateD2D1Resources(const HWND& hwnd)
{
	HRESULT result = D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		mFactory.GetAddressOf());
	if (FAILED(result)) return false;

	RECT rect{};
	GetClientRect(hwnd, &rect);

	result = mFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(
			hwnd, 
			D2D1::SizeU(
				rect.right - rect.left,
				rect.bottom - rect.top)),
		mRenderTarget.GetAddressOf());
	if (FAILED(result)) return false;

	result = mRenderTarget->CreateLayer(mLayer.GetAddressOf());
	if (FAILED(result)) return false;
	
	return true;
}

bool GraphicScene::CreateDirectWriteResources()
{
	HRESULT hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(mWriteFactory.GetAddressOf()));
	if (FAILED(hr)) return false;

	hr = mWriteFactory->CreateTextFormat(
		L"D2DCoding",
		NULL,
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		30.0f, L"ko-kr", mTextFormat.GetAddressOf());
	if (FAILED(hr)) return false;

	if(FAILED(mTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER))) return false;
	if(FAILED(mTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER))) return false;

	return true;
}
	
void GraphicScene::BuildMap()
{
	for (int i = 0; i < 1; i++)
	{
		for (int j = 0; j < 1; j++)
		{
			auto boxObj = std::make_unique<GameObject>(mRenderTarget.Get());
			boxObj->SetShape(std::make_unique<Rect>(D2D1::SizeF(800.0f, 800.0f)));
			boxObj->SetImage(mBitmaps[0]->GetBitmapResource());
			boxObj->SetPosition({ i * 400.0f + 400.0f, j * 400.0f + 400.0f });
			mGameObjects[Layer::STATIC].push_back(move(boxObj));
		}
	}
}

void GraphicScene::BuildImages()
{
	mBitmaps[0]->DecodeBitmap(mRenderTarget.Get(), L"Image\\chessboard.png");


}

void GraphicScene::Resize(const HWND& hwnd)
{
	RECT rect{};
	GetClientRect(hwnd, &rect);

	HRESULT result = mRenderTarget->Resize(
		D2D1::SizeU(rect.right - rect.left, rect.bottom - rect.top));
	if (FAILED(result)) throw D2DException("Failed to resize");
}

void GraphicScene::Draw()
{
	mRenderTarget->BeginDraw();
	
	mRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
	mRenderTarget->Clear(ColorF(ColorF::White));

	for (const auto& list : mGameObjects)
		for (const auto& obj : list)
			obj->Draw(mRenderTarget.Get(), mTextFormat.Get());	
	
	mRenderTarget->EndDraw();
}

void GraphicScene::Update(const float elapsed)
{
	OnProcessKeyInput(elapsed);

	for (const auto& list : mGameObjects)
		for(const auto& obj : list)
			obj->Update(elapsed);
}

void GraphicScene::OnProcessKeyInput(const float elapsed)
{
	if (!mPlayer) return;

	const float moveSpeed = 200.0f;
	if (GetAsyncKeyState(VK_LEFT) & 0x8000)
	{
		mPlayer->Move(-moveSpeed * elapsed, 0.0f);
	}

	else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
	{
		mPlayer->Move(moveSpeed * elapsed, 0.0f);
	}

	else if (GetAsyncKeyState(VK_UP) & 0x8000)
	{
		mPlayer->Move(0.0f, moveSpeed * elapsed);
	}

	else if (GetAsyncKeyState(VK_DOWN) & 0x8000)
	{
		mPlayer->Move(0.0f, -moveSpeed * elapsed);
	}

	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	{
		mPlayer->Scale(2.0f, 1.0f);
	}

}

void GraphicScene::OnProcessMouseInput(UINT msg, WPARAM wParam, LPARAM lParam)
{
	
}




