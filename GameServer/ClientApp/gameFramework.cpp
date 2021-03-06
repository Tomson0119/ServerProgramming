#include "stdafx.h"
#include "gameFramework.h"
#include "camera.h"
#include "clientSocket.h"

using namespace std;

GameFramework::GameFramework()
	: D3DFramework()
{
}

GameFramework::~GameFramework()
{
	size_t size = mScenes.size();
	for (int i = 0; i < size; ++i) mScenes.pop();
}

bool GameFramework::InitFramework()
{
	if (!D3DFramework::InitFramework())
		return false;

	// 초기화하는 명령어를 넣기 위해 커맨드 리스트를 개방한다.
	ThrowIfFailed(mCommandList->Reset(mCommandAllocator.Get(), nullptr));

	mCamera = make_unique<Camera>();
	mCamera->SetPosition(0.0f, 0.0f, 0.0f);
	mCamera->SetLens(0.25f * Math::PI, GetAspect(), 1.0f, 1000.0f);
	mCamera->Pitch(90.0f);

	mScenes.push(make_unique<GameScene>());
	mScenes.top()->BuildD2DResources(m_hwnd);
	mScenes.top()->BuildObjects(mD3dDevice.Get(), mCommandList.Get());

	// Command List를 닫고 Queue에 명령어를 싣는다.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdList[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);

	// 초기화가 진행될 때까지 기다린다.
	WaitUntilGPUComplete();

	return true;
}

void GameFramework::OnResize()
{
	D3DFramework::OnResize();
	if(mCamera) mCamera->SetLens(GetAspect());
}

void GameFramework::OnProcessMouseDown(WPARAM buttonState, int x, int y)
{
	if (!mScenes.empty()) mScenes.top()->OnProcessMouseDown(buttonState, x, y);
}

void GameFramework::OnProcessMouseUp(WPARAM buttonState, int x, int y)
{
	ReleaseCapture();
	if (!mScenes.empty()) mScenes.top()->OnProcessMouseUp(buttonState, x, y);
}

void GameFramework::OnProcessMouseMove(WPARAM buttonState, int x, int y)
{
	if (!mScenes.empty()) mScenes.top()->OnProcessMouseMove(buttonState);
}

void GameFramework::OnProcessKeyInput(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_ESCAPE:
			if (mScenes.size() > 0)
				mScenes.pop();

			if (mScenes.empty()) {
				mClientSck->Disconnect();
				PostQuitMessage(0);
				return;
			}
			break;

		case VK_F9:
			D3DFramework::ChangeFullScreenState();
			break;
		}
		break;

	case WM_KEYDOWN:
		if (0x25 <= wParam && wParam <= 0x28)
		{
			mClientSck->SendMovePacket((char)wParam);
		}
		break;
	}
	if (!mScenes.empty()) mScenes.top()->OnProcessKeyInput(uMsg, wParam, lParam);
}

void GameFramework::Update()
{
	D3DFramework::UpdateFrameStates();

	XMFLOAT3& player_pos = mScenes.top()->GetPlayerPosition(mClientSck->ID);
	if (!Vector3::Equal(player_pos, XMFLOAT3(-100.0f, -100.0f, -100.0f)))
	{
		mCamera->SetPosition(player_pos.x + 0.5f, player_pos.y + 19.2f, player_pos.z + 0.5f);
	}
	mCamera->Update(mTimer.ElapsedTime());
	if (!mScenes.empty())
	{
		mClientSck->PlayerInfoLock.lock();
		const auto copyCoord = mClientSck->PlayerInfos;
		mClientSck->PlayerInfoLock.unlock();

		if (mClientSck->Dirty)
		{
			mScenes.top()->AppendOrDeletePlayers(mD3dDevice.Get(), mClientSck->ID, copyCoord);
			mClientSck->Dirty = false;
		}
		mScenes.top()->UpdatePlayersCoord(copyCoord);
		mScenes.top()->Update(mTimer);
		mScenes.top()->UpdateConstants(mCamera.get());
	}
}

void GameFramework::Draw()
{
	// 명령어 할당자를 먼저 초기화해준다.
	ThrowIfFailed(mCommandAllocator->Reset());

	// Command List를 Pipeline State로 묶는다. 
	ThrowIfFailed(mCommandList->Reset(mCommandAllocator.Get(), nullptr));

	mCommandList->RSSetViewports(1, &mViewPort);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// 화면 버퍼의 상태를 Render Target 상태로 전이한다.
	mCommandList->ResourceBarrier(1, &Extension::ResourceBarrier(
		CurrentBackBuffer(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	
	// 화면 버퍼와 깊이 스텐실 버퍼를 초기화한다.
	XMFLOAT4 color = (!mScenes.empty()) ? mScenes.top()->GetFrameColor() : (XMFLOAT4)Colors::White;

	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), (FLOAT*)&color, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
	
	// 렌더링할 버퍼를 구체적으로 설정한다.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), TRUE, &DepthStencilView());

	if (!mScenes.empty()) mScenes.top()->Draw(mCommandList.Get());

	// 화면 버퍼의 상태를 다시 PRESENT 상태로 전이한다.
	mCommandList->ResourceBarrier(1, &Extension::ResourceBarrier(
		CurrentBackBuffer(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	
	ThrowIfFailed(mCommandList->Close());

	ID3D12CommandList* cmdList[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdList), cmdList);

	// 커맨드 리스트의 명령어들을 다 실행하기까지 기다린다.
	WaitUntilGPUComplete();

	ThrowIfFailed(mSwapChain->Present(0, 0));  // 화면버퍼를 Swap한다.

	// 다음 후면버퍼 위치로 이동한 후 다시 기다린다.
	mCurrBackBufferIndex = mSwapChain->GetCurrentBackBufferIndex();
	WaitUntilGPUComplete();
}
