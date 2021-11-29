#include "stdafx.h"
#include "gameScene.h"

using namespace std;

GameScene::GameScene()
{
	mUILayer = make_unique<UILayer>();
}

GameScene::~GameScene()
{
}

void GameScene::BuildObjects(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	BuildRootSignature(device);
	BuildShadersAndPSOs(device);
	BuildTextures(device, cmdList);
	BuildGameObjects(device, cmdList);
	BuildConstantBuffers(device);
	BuildDescriptorHeap(device);
}

void GameScene::BuildD2DResources(HWND hwnd)
{
	ComPtr<ID2D1Factory> pFactory;
	ThrowIfFailed(D2D1CreateFactory(
		D2D1_FACTORY_TYPE_SINGLE_THREADED,
		pFactory.GetAddressOf()));

	RECT rect{};
	GetClientRect(hwnd, &rect);

	ThrowIfFailed(pFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(
			hwnd,
			D2D1::SizeU(
				rect.right - rect.left,
				rect.bottom - rect.top)),
		mD2DRenderTarget.GetAddressOf()));

	BuildDWriteResources();
}

void GameScene::UpdateConstants(Camera* camera)
{
	// 카메라로부터 상수를 받는다.
	mCameraCB->CopyData(0, camera->GetConstants());

	// 광원과 관련된 상수버퍼를 초기화 및 업데이트한다.
	LightConstants lightCnst;
	lightCnst.Ambient = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);

	lightCnst.Lights[0].Diffuse = { 0.5f, 0.5f, 0.5f };
	lightCnst.Lights[0].Direction = { -1.0f, 1.0f, -1.0f };

	lightCnst.Lights[1].Diffuse = { 0.15f, 0.15f, 0.15f };
	lightCnst.Lights[1].Direction = { 0.0f, 1.0f, 1.0f };

	lightCnst.Lights[2].Diffuse = { 0.35f, 0.35f, 0.35f };
	lightCnst.Lights[2].Direction = { 1.0f, 1.0f, -1.0f };

	mLightCB->CopyData(0, lightCnst);

	for (const auto& [_, pso] : mPipelines)
		pso->UpdateConstants();
}

void GameScene::Update(const GameTimer& timer)
{
	OnPreciseKeyInput(timer);

	for (const auto& [_, pso] : mPipelines)
		pso->Update(timer.ElapsedTime());
}

void GameScene::UpdatePlayersCoord(const std::unordered_map<int, PlayerInfo>& coords)
{
	for (const auto& coord : coords)
	{
		float playerPosX = (float)coord.second.x + 0.5f;
		float playerPosZ = (float)coord.second.y + 0.5f;
		mPlayers[coord.first]->SetPosition(playerPosX, 0.0f, playerPosZ);

		int len = strlen(coord.second.message);
		if(len> 0)
			mPlayers[coord.first]->SetText(std::wstring(&coord.second.message[0], &coord.second.message[len-1]));
	}
}

void GameScene::AppendOrDeletePlayers(
	ID3D12Device* device, int myID, 
	const std::unordered_map<int, PlayerInfo>& coords)
{
	for (const auto& coord : coords)
	{
		if (mPlayers.find(coord.first) != mPlayers.end())
			continue;

		auto player = make_shared<GameObject>();
		player->SetMesh(mPawnMesh);
		if (coord.first == myID)
			player->SetSRVIndex(1);
		else if (coord.first < NPC_ID_START)
			player->SetSRVIndex(2);
		else
			player->SetSRVIndex(3);
		player->SetPosition(0.0f, 0.0f, 0.0f);
		player->Scale(0.5f, 0.5f, 0.5f);
		player->SetMaterial(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.01f, 0.01f, 0.01f), 0.25f);
		mPipelines["texLit"]->AppendObject(player);

		mPlayers[coord.first] = player.get();
	}

	vector<int> trash_can;
	for (const auto& player : mPlayers)
	{
		if (coords.find(player.first) != coords.end())
			continue;

		mPipelines["texLit"]->DeleteObject(player.second);
		trash_can.push_back(player.first);
	}

	for (int e : trash_can)
		mPlayers.erase(e);

	mPipelines["texLit"]->BuildConstantBuffer(device);
	mPipelines["texLit"]->BuildDescriptorHeap(device, 2, 3);
}

void GameScene::Draw(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetGraphicsRootSignature(mRootSignature.Get());	
	cmdList->SetGraphicsRootConstantBufferView(0, mCameraCB->GetGPUVirtualAddress());
	cmdList->SetGraphicsRootConstantBufferView(1, mLightCB->GetGPUVirtualAddress());
	
	mPipelines["texLit"]->SetAndDraw(cmdList);
	mPipelines["texLit"]->DrawTexts(mD2DRenderTarget.Get(), mTextFormat.Get(), mColorBrush.Get());
}

void GameScene::OnProcessKeyInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
}

XMFLOAT3 GameScene::GetPlayerPosition(int id)
{
	if (mPlayers.find(id) != mPlayers.end())
		return mPlayers[id]->GetPosition();
	return XMFLOAT3(-100.0f, -100.0f, -100.0f);
}

void GameScene::BuildRootSignature(ID3D12Device* device)
{
	D3D12_DESCRIPTOR_RANGE descRanges[2];
	descRanges[0] = Extension::DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
	descRanges[1] = Extension::DescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);	

	D3D12_ROOT_PARAMETER parameters[4];
	parameters[0] = Extension::Descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, 0, D3D12_SHADER_VISIBILITY_ALL);  // CameraCB
	parameters[1] = Extension::Descriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, 1, D3D12_SHADER_VISIBILITY_ALL);  // LightCB
	parameters[2] = Extension::DescriptorTable(1, &descRanges[0], D3D12_SHADER_VISIBILITY_ALL);			   // Object,  CBV
	parameters[3] = Extension::DescriptorTable(1, &descRanges[1], D3D12_SHADER_VISIBILITY_PIXEL);		   // Texture, SRV

	D3D12_STATIC_SAMPLER_DESC samplerDesc = Extension::SamplerDesc(0, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP);
	D3D12_ROOT_SIGNATURE_DESC rootSigDesc = Extension::RootSignatureDesc(_countof(parameters), parameters, 
		1, &samplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> rootSigBlob = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;

	ThrowIfFailed(D3D12SerializeRootSignature(
		&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		rootSigBlob.GetAddressOf(), errorBlob.GetAddressOf()));

	ThrowIfFailed(device->CreateRootSignature(
		0, rootSigBlob->GetBufferPointer(),
		rootSigBlob->GetBufferSize(),
		IID_PPV_ARGS(&mRootSignature)));
}

void GameScene::BuildShadersAndPSOs(ID3D12Device* device)
{
	auto texShader = make_unique<DefaultShader>(L"Shaders\\texShader.hlsl");

	mPipelines["texLit"] = make_unique<Pipeline>();
	mPipelines["texLit"]->BuildPipeline(device, mRootSignature.Get(), texShader.get());
}

void GameScene::BuildDescriptorHeap(ID3D12Device* device)
{
	for (const auto& [_, pso] : mPipelines)
		pso->BuildDescriptorHeap(device, 2, 3);
}

void GameScene::BuildDWriteResources()
{
	ThrowIfFailed(mD2DRenderTarget->CreateSolidColorBrush(
		D2D1::ColorF(1.0f, 0.0f, 0.0f, 1.0f), mColorBrush.GetAddressOf()));

	ThrowIfFailed(DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(mWriteFactory.GetAddressOf())));

	ThrowIfFailed(mWriteFactory->CreateTextFormat(
		L"D2DCoding",
		NULL,
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		30.0f, L"ko-kr", mTextFormat.GetAddressOf()));

	ThrowIfFailed(mTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
	ThrowIfFailed(mTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
}

void GameScene::BuildTextures(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	auto boardTex = make_shared<Texture>();
	boardTex->CreateTextureResource(device, cmdList, L"Resources\\chessboard.dds");
	boardTex->SetDimension(D3D12_SRV_DIMENSION_TEXTURE2D);
	mPipelines["texLit"]->AppendTexture(boardTex);

	auto whiteTex = make_shared<Texture>();
	whiteTex->CreateTextureResource(device, cmdList, L"Resources\\white.dds");
	whiteTex->SetDimension(D3D12_SRV_DIMENSION_TEXTURE2D);
	mPipelines["texLit"]->AppendTexture(whiteTex);

	auto blackTex = make_shared<Texture>();
	blackTex->CreateTextureResource(device, cmdList, L"Resources\\black.dds");
	blackTex->SetDimension(D3D12_SRV_DIMENSION_TEXTURE2D);
	mPipelines["texLit"]->AppendTexture(blackTex);

	auto iceTex = make_shared<Texture>();
	iceTex->CreateTextureResource(device, cmdList, L"Resources\\ice.dds");
	iceTex->SetDimension(D3D12_SRV_DIMENSION_TEXTURE2D);
	mPipelines["texLit"]->AppendTexture(iceTex);
}

void GameScene::BuildGameObjects(ID3D12Device* device, ID3D12GraphicsCommandList* cmdList)
{
	mPawnMesh = make_shared<Mesh>();
	mPawnMesh->LoadFromBinary(device, cmdList, L"Models\\pawn.bin");

	const float meshWidth = 8.0f * (WORLD_WIDTH / 80);
	const float meshDepth = 8.0f * (WORLD_HEIGHT / 80);
	auto gridMesh = make_shared<GridMesh>(device, cmdList, meshWidth, meshDepth, 8.0f, 8.0f);

	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			auto tile = make_shared<GameObject>();
			tile->SetMesh(gridMesh);
			tile->SetPosition(XMFLOAT3(j * 100 + 100, 0.0f, i * 100.0f + 100));
			tile->SetSRVIndex(0);
			tile->SetMaterial(XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.01f, 0.01f, 0.01f), 0.25f);
			mPipelines["texLit"]->AppendObject(tile);
		}
	}
}

void GameScene::BuildConstantBuffers(ID3D12Device* device)
{
	mCameraCB = std::make_unique<ConstantBuffer<CameraConstants>>(device, 1);
	mLightCB = std::make_unique<ConstantBuffer<LightConstants>>(device, 1);

	for (const auto& [_, pso] : mPipelines)
		pso->BuildConstantBuffer(device);
}
