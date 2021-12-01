#include "stdafx.h"
#include "Graphics.h"
#include "GameObject.h"
#include "BitmapLoader.h"
#include "Shape.h"

using namespace std;


GraphicScene::GraphicScene()
	: mCameraMatrix(D2D1::Matrix3x2F::Identity()),
	  mCameraPosition(D2D1::Point2F(0.0f, 0.0f)),
	  mPlayer(nullptr), mPlayerOffset(D2D1::Point2F(0.0f, 0.0f))
{
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
	BuildMap();
	return true;
}

bool GraphicScene::CreateD2D1Resources(const HWND& hwnd)
{
	ThrowIfFailed(D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		mFactory.GetAddressOf()));

	RECT rect{};
	GetClientRect(hwnd, &rect);

	ThrowIfFailed(mFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(
			hwnd,
			D2D1::SizeU(
				rect.right - rect.left,
				rect.bottom - rect.top)),
		mRenderTarget.GetAddressOf()));

	ThrowIfFailed(mRenderTarget->CreateLayer(mLayer.GetAddressOf()));

	ThrowIfFailed(mRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f), mIDLabelColorBrush.GetAddressOf()));
	ThrowIfFailed(mRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f), mChatLabelColorBrush.GetAddressOf()));
	return true;
}

bool GraphicScene::CreateDirectWriteResources()
{	
	ThrowIfFailed(DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(mWriteFactory.GetAddressOf())));

	ThrowIfFailed(mWriteFactory->CreateTextFormat(
		L"D2DCoding",
		NULL,
		DWRITE_FONT_WEIGHT_BOLD,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		16.0f, L"ko-kr", mTextFormat.GetAddressOf()));

	ThrowIfFailed(mTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
	ThrowIfFailed(mTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));

	return true;
}
	
void GraphicScene::BuildMap()
{	
	OutputDebugString(L"Build Map\n");

	float rect_size = 2880.0f;
	float h_size = rect_size * 0.5f;
	mPlayerOffset = { h_size / 64, h_size / 64 };

	for (int i = 0; i < WORLD_HEIGHT/64+1; i++)
	{
		for (int j = 0; j < WORLD_WIDTH/64+1; j++)
		{
			auto board = std::make_unique<GameObject>(mRenderTarget.Get());
			board->SetShape(std::make_unique<Rect>(D2D1::SizeF(rect_size, rect_size)));
			board->SetImage(mBitmaps[0]->GetBitmapResource());
			board->SetCamera(&mCameraMatrix);
			board->SetPosition({ i * h_size * 2 + h_size, j * h_size * 2 + h_size });
			mStaticObjects.push_back(move(board));
		}
	}
}

void GraphicScene::BuildImages()
{
	mBitmaps.push_back(std::make_unique<BitmapLoader>());
	if (mBitmaps[0]->DecodeBitmap(mRenderTarget.Get(), L"Image\\chessboard.png") == false)
		throw D2DException(L"Failed to Decode image\n");
}

void GraphicScene::InitializePlayer(int id, char* name, short x, short y)
{
	CreateNewObject(id, 0, name, x, y);
	mPlayer = mMovingObjects[id].get();
}

void GraphicScene::CreateNewObject(int id, char obj_type, char* name, short x, short y)
{
	mMovingObjects[id] = std::make_unique<GameObject>(mRenderTarget.Get());
	mMovingObjects[id]->SetShape(std::make_unique<Circle>(16.0f));
	mMovingObjects[id]->SetColor(mRenderTarget.Get(), D2D1::ColorF(1.0f, 0.0f, 0.0f, 1.0f));
	mMovingObjects[id]->SetID(CharToWString(name));
	mMovingObjects[id]->SetCamera(&mCameraMatrix);
	mMovingObjects[id]->SetPosition({ 
		(float)x * mPlayerOffset.x * 2 + mPlayerOffset.x, 
		(float)y * mPlayerOffset.y * 2 + mPlayerOffset.y });
}

void GraphicScene::UpdatePlayerPosition(int id, short x, short y)
{
	if (mMovingObjects.find(id) == mMovingObjects.end())
	{
		std::stringstream ss;
		ss << "Can't update position to object [ID " << id << "]\n";
		OutputDebugStringA(ss.str().c_str());
		return;
	}
	
	mMovingObjects[id]->SetPosition({
		(float)x * mPlayerOffset.x * 2 + mPlayerOffset.x,
		(float)y * mPlayerOffset.y * 2 + mPlayerOffset.y });
}

void GraphicScene::UpdatePlayerChat(int id, char* msg)
{
	if (mMovingObjects.find(id) == mMovingObjects.end())
	{
		std::stringstream ss;
		ss << "Can't set chat message to object [ID " << id << "]\n";
		OutputDebugStringA(ss.str().c_str());
		return;
	}
	mMovingObjects[id]->SetChat(CharToWString(msg));
}

void GraphicScene::EraseObject(int id)
{
	if (mMovingObjects.find(id) == mMovingObjects.end())
	{
		std::stringstream ss;
		ss << "Can't erase object [ID " << id << "]\n";
		OutputDebugStringA(ss.str().c_str());
		return;
	}
	
	mMovingObjects.erase(id);
}

void GraphicScene::Resize(const HWND& hwnd)
{
	RECT rect{};
	GetClientRect(hwnd, &rect);

	ThrowIfFailed(mRenderTarget->Resize(
		D2D1::SizeU(rect.right - rect.left, rect.bottom - rect.top)));
}

void GraphicScene::Draw()
{
	mRenderTarget->BeginDraw();
	
	mRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
	mRenderTarget->Clear(ColorF(ColorF::White));

	for (const auto& staticObj : mStaticObjects)
		staticObj->Draw(mRenderTarget.Get(), mTextFormat.Get());
	for (const auto& [_, movingObj] : mMovingObjects) {
		movingObj->Draw(mRenderTarget.Get(), mTextFormat.Get());
		movingObj->DrawIDLabel(mRenderTarget.Get(), mTextFormat.Get(), mIDLabelColorBrush.Get());
		movingObj->DrawChatLabel(mRenderTarget.Get(), mTextFormat.Get(), mChatLabelColorBrush.Get());
	}	
	mRenderTarget->EndDraw();
}

void GraphicScene::Update(const float elapsed)
{
	OnProcessKeyInput(elapsed);

	D2D1_POINT_2F player_pos = mPlayer->GetPosition();
	D2D1_SIZE_F rt_size = mRenderTarget->GetSize();
	
	float player_pos_x = player_pos.x - rt_size.width * 0.5f;
	float player_pos_y = player_pos.y - rt_size.height * 0.5f;
	mCameraMatrix = D2D1::Matrix3x2F::Translation(-player_pos_x, -player_pos_y);

	for (const auto& staticObj : mStaticObjects)
		staticObj->Update(elapsed);
	for (const auto& [_, movingObj] : mMovingObjects)
		movingObj->Update(elapsed);
}

