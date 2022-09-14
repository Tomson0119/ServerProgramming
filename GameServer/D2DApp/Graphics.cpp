#include "stdafx.h"
#include "Graphics.h"
#include "GameObject.h"
#include "BitmapLoader.h"
#include "Shape.h"

#include <sstream>

using namespace std;

GraphicScene::GraphicScene()
	: mCameraMatrix(D2D1::Matrix3x2F::Identity()),
	  mCameraPosition(D2D1::Point2F(0.0f, 0.0f)),
	  mPlayerOffset(D2D1::Point2F(0.0f, 0.0f)),
	  mPlayerID(0), mChatShowedTime(0.0f), mAppHwnd()
{
}

GraphicScene::~GraphicScene()
{
}

bool GraphicScene::Init(const HWND& hwnd)
{
	mAppHwnd = hwnd;

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
		D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f), mLabelColorBrush.GetAddressOf()));
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
		20.0f, L"ko-kr", mTextFormat.GetAddressOf()));

	ThrowIfFailed(mTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
	ThrowIfFailed(mTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));

	return true;
}
	
void GraphicScene::BuildMap()
{
	float rect_size = 2368.0f;
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

void GraphicScene::InitializePlayer(sc_packet_login_ok& player_info, const char* name)
{
	auto player = std::make_unique<Player>(mRenderTarget.Get());
	player->SetInfo(player_info.level, player_info.hp, player_info.maxhp, player_info.exp);
	player->SetShape(std::make_unique<Circle>(16.0f));
	player->SetColor(mRenderTarget.Get(), D2D1::ColorF(1.0f, 0.0f, 0.0f, 1.0f));
	player->SetID(CharToWString(name));
	player->SetCamera(&mCameraMatrix);
	player->SetCoord(player_info.x, player_info.y);
	player->SetPosition({
		(float)player_info.x * mPlayerOffset.x * 2 + mPlayerOffset.x,
		(float)player_info.y * mPlayerOffset.y * 2 + mPlayerOffset.y });
	mMovingObjectsLock.lock();
	mMovingObjects[player_info.id] = std::move(player);
	mMovingObjectsLock.unlock();
	mPlayerID = player_info.id;
}

void GraphicScene::CreateNewObject(int id, char obj_type, const char* name, short x, short y)
{
	auto newObj = std::make_unique<GameObject>(mRenderTarget.Get());
	newObj->SetShape(std::make_unique<Circle>(16.0f));
	
	switch (obj_type)
	{
	case 0:
		newObj->SetColor(mRenderTarget.Get(), D2D1::ColorF(1.0f, 0.0f, 0.0f, 1.0f));
		break;

	case 1:
		newObj->SetColor(mRenderTarget.Get(), D2D1::ColorF(0.0f, 1.0f, 0.0f, 1.0f));
		break;
	}
	newObj->SetID(CharToWString(name));
	newObj->SetCamera(&mCameraMatrix);
	newObj->SetCoord(x, y);
	newObj->SetPosition({
		(float)x * mPlayerOffset.x * 2 + mPlayerOffset.x, 
		(float)y * mPlayerOffset.y * 2 + mPlayerOffset.y });

	mMovingObjectsLock.lock();
	mMovingObjects[id] = std::move(newObj);
	mMovingObjectsLock.unlock();
}

void GraphicScene::UpdatePlayerPosition(int id, short x, short y)
{
	mMovingObjectsLock.lock();
	if(FindMovingObjectID(id))
	{
		mMovingObjects[id]->SetCoord(x, y);
		mMovingObjects[id]->SetPosition({
			(float)x * mPlayerOffset.x * 2 + mPlayerOffset.x,
			(float)y * mPlayerOffset.y * 2 + mPlayerOffset.y });
	}
	mMovingObjectsLock.unlock();
}

void GraphicScene::UpdatePlayerChat(int id, char* msg)
{
	mMovingObjectsLock.lock();
	if(FindMovingObjectID(id))
		mMovingObjects[id]->SetChat(CharToWString(msg));
	mMovingObjectsLock.unlock();
}

void GraphicScene::UpdatePlayerStatus(sc_packet_status_change& status)
{
	mMovingObjectsLock.lock();
	if (FindMovingObjectID(mPlayerID))
	{
		mMovingObjects[mPlayerID]->SetInfo(
			status.level, status.hp,
			status.maxhp, status.exp);
	}
	mMovingObjectsLock.unlock();
}

void GraphicScene::EraseObject(int id)
{
	mMovingObjectsLock.lock();
	if(FindMovingObjectID(id))	
		mMovingObjects.erase(id);
	mMovingObjectsLock.unlock();
}

bool GraphicScene::FindMovingObjectID(int id)
{
	bool ret = true;
	if (mMovingObjects.find(id) == mMovingObjects.end())
	{
		std::stringstream ss;
		ss << "Can't find object ID [" << id << "]\n";
		OutputDebugStringA(ss.str().c_str());
		ret = false;
	}
	return ret;
}

void GraphicScene::CreateAttackArea()
{
	float rect_size = 2368.0f;
	float h_size = rect_size * 0.5f;

	auto playerCoord = mMovingObjects[mPlayerID]->GetCoord();

	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			auto board = std::make_unique<GameObject>(mRenderTarget.Get());
			board->SetShape(std::make_unique<Rect>(D2D1::SizeF(rect_size / 64.0f, rect_size / 64.0f)));
			board->SetColor(mRenderTarget.Get(), D2D1::ColorF(0.0f,0.0f,1.0f,0.3f));
			board->SetCamera(&mCameraMatrix);
			board->SetPosition({
				(float)(playerCoord.first + j - 1)* mPlayerOffset.x * 2 + mPlayerOffset.x, 
				(float)(playerCoord.second + i - 1)* mPlayerOffset.y * 2 + mPlayerOffset.y });
			board->SetDurationTime(600ms);
			mEffects.insert(move(board));
		}
	}
}

void GraphicScene::Quit()
{
	PostMessage(mAppHwnd, WM_CLOSE, NULL, NULL);
}

std::wstring GraphicScene::GetPlayerName(int id)
{
	mMovingObjectsLock.lock();
	if (FindMovingObjectID(id))
	{
		mMovingObjectsLock.unlock();
		return mMovingObjects[id]->GetID();
	}
	mMovingObjectsLock.unlock();
	return {};
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
		if(staticObj) staticObj->Draw(mRenderTarget.Get(), mTextFormat.Get(), mLabelColorBrush.Get());
	
	for (const auto& effect : mEffects)
		if (effect) effect->Draw(mRenderTarget.Get(), mTextFormat.Get(), mLabelColorBrush.Get());

	mMovingObjectsLock.lock();
	for (const auto& [_, movingObj] : mMovingObjects)
		if(movingObj) movingObj->Draw(mRenderTarget.Get(), mTextFormat.Get(), mLabelColorBrush.Get());
	mMovingObjectsLock.unlock();

	mRenderTarget->EndDraw();
}

void GraphicScene::Update(const float elapsed)
{
	OnProcessKeyInput(elapsed);

	if (mMovingObjects[mPlayerID] == nullptr) return;

	D2D1_POINT_2F player_pos = mMovingObjects[mPlayerID]->GetPosition();
	D2D1_SIZE_F rt_size = mRenderTarget->GetSize();

	float player_pos_x = player_pos.x - rt_size.width * 0.5f;
	float player_pos_y = player_pos.y - rt_size.height * 0.5f;
	mCameraMatrix = D2D1::Matrix3x2F::Translation(-player_pos_x, -player_pos_y);

	for (const auto& staticObj : mStaticObjects)
		staticObj->Update(elapsed);
	for (const auto& [_, movingObj] : mMovingObjects)
		movingObj->Update(elapsed);

	EraseTimeOutObjects();
}

void GraphicScene::EraseTimeOutObjects()
{
	for (auto iter = mEffects.begin();iter!=mEffects.end();)
	{
		if ((*iter)->IsTimeOut()) iter = mEffects.erase(iter);
		else iter++;
	}
}
