#include "stdafx.h"
#include "NetClient.h"
#include "Graphics.h"
#include "ChatWindow.h"
#include "LogWindow.h"

NetClient::NetClient()
	: Socket(), mLoop(true),
	  mIOCP{}, mScene(), mChatWin(), mLogWin()
{
	Init();
}

NetClient::~NetClient()
{
	if(mSocketThread.joinable())
		mSocketThread.join();
}

void NetClient::SetInterfaces(GraphicScene* scene, ChatWindow* chatWin, LogWindow* logWin)
{
	mScene = scene;
	mChatWin = chatWin;
	mLogWin = logWin;
}

void NetClient::Start(const std::string& name)
{
	mUsername = name;

	mIOCP.RegisterDevice(mSocket, 0);
	mSocketThread = std::thread{ NetClient::NetworkThreadFunc, std::ref(*this) };
	RecvMsg();
	SendLoginPacket(name.c_str());
}

void NetClient::SendLoginPacket(const char* name)
{
	OutputDebugString(L"Send login packet.\n");
	cs_packet_login login_packet{};
	login_packet.size = sizeof(cs_packet_login);
	login_packet.type = CS_PACKET_LOGIN;
	strncpy_s(login_packet.name, name, strlen(name));
	SendMsg(reinterpret_cast<std::byte*>(&login_packet), login_packet.size);
}

void NetClient::OnProcessKeyInput(char input)
{
	if (0x25 <= input && input <= 0x28)
		SendMovePacket(input);
	else if (input == 'A') {
		SendAttackPacket();
	}
}

void NetClient::SendMovePacket(char input)
{
	cs_packet_move move_packet{};
	move_packet.size = sizeof(cs_packet_move);
	move_packet.type = CS_PACKET_MOVE;
	char dir = -1;
	switch (input)
	{
	case 0x25:
		dir = 2;
		break;
	case 0x26:
		dir = 0;
		break;
	case 0x27:
		dir = 3;
		break;
	case 0x28:
		dir = 1;
		break;
	}
	move_packet.direction = dir;
	SendMsg(reinterpret_cast<std::byte*>(&move_packet), move_packet.size);
}

void NetClient::SendAttackPacket()
{
	cs_packet_attack attack_packet{};
	attack_packet.size = sizeof(cs_packet_attack);
	attack_packet.type = CS_PACKET_ATTACK;
	SendMsg(reinterpret_cast<std::byte*>(&attack_packet), attack_packet.size);
}

void NetClient::SendChatPacket(const char* msg)
{
	cs_packet_chat chat_packet{};
	chat_packet.size = sizeof(cs_packet_chat);
	chat_packet.type = CS_PACKET_CHAT;
	strncpy_s(chat_packet.message, msg, strlen(msg));
	SendMsg(reinterpret_cast<std::byte*>(&chat_packet), chat_packet.size);
}

void NetClient::Disconnect()
{
	mLoop = false;
	mIOCP.PostToCompletionQueue(nullptr, 0);
}

void NetClient::NetworkThreadFunc(NetClient& client)
{
	try {
		if (FAILED(CoInitializeEx(NULL, COINIT_MULTITHREADED)))
			throw NetException("CoInitializeEx failed");
		
		while (client.mLoop)
		{
			CompletionInfo info = client.mIOCP.GetCompletionInfo();
			WSAOVERLAPPEDEX* over_ex = reinterpret_cast<WSAOVERLAPPEDEX*>(info.overEx);
			
			if (over_ex == nullptr)
				break;

			if (info.success == FALSE)
			{
				if (over_ex->Operation == OP::SEND)
					delete over_ex;
				break;
			}

			client.HandleCompletionInfo(over_ex, info.bytes);			
		}
	}
	catch (std::exception& ex)
	{
		MessageBoxA(NULL, ex.what(), "Error", MB_OK);
	}
	CoUninitialize();
}

void NetClient::SendMsg(std::byte* msg, int bytes)
{
	WSAOVERLAPPEDEX* send_over = new WSAOVERLAPPEDEX(OP::SEND, msg, bytes);
	Send(*send_over);
}

void NetClient::RecvMsg()
{
	mRecvOverlapped.Reset(OP::RECV);
	Recv(mRecvOverlapped);
}

void NetClient::HandleCompletionInfo(WSAOVERLAPPEDEX* over, int bytes)
{
	switch (over->Operation)
	{
	case OP::RECV:
	{
		if (bytes == 0)
		{
			Disconnect();
			break;
		}
		over->NetBuffer.ShiftWritePtr(bytes);
		ProcessPackets(over, bytes);
		RecvMsg();
		break;
	}
	case OP::SEND:
	{
		if (bytes != over->WSABuffer.len)
			Disconnect();
		delete over;
		break;
	}
	}
}

void NetClient::ProcessPackets(WSAOVERLAPPEDEX* over, int bytes)
{
	while (over->NetBuffer.Readable())
	{
		std::byte* packet = over->NetBuffer.BufReadPtr();
		unsigned char type = static_cast<unsigned char>(GetPacketType(packet));

		switch (type)
		{
		case SC_PACKET_LOGIN_OK:
		{
			sc_packet_login_ok* login_packet = reinterpret_cast<sc_packet_login_ok*>(packet);
			mScene->InitializePlayer(*login_packet, mUsername.c_str());
			break;
		}
		case SC_PACKET_LOGIN_FAIL:
		{
			sc_packet_login_fail* fail_packet = reinterpret_cast<sc_packet_login_fail*>(packet);
			std::wstring str = L"Login failed: ";
			
			switch (fail_packet->reason)
			{
			case -1:
				str += L"일치하는 ID가 존재하지 않습니다.";
				break;
			case 0:
				str += L"이미 해당 ID로 접속한 기록이 있습니다.";
				break;
			case 1:
				str += L"최대 유저 접속 수에 도달하여 로그인이 거부되었습니다.";
				break;
			}

			MessageBoxW(NULL, str.c_str(), L"Login Error", MB_OK);
			mScene->Quit();
			Disconnect();
			return;
		}
		case SC_PACKET_MOVE:
		{
			sc_packet_move* move_pck = reinterpret_cast<sc_packet_move*>(packet);
			mScene->UpdatePlayerPosition(move_pck->id, move_pck->x, move_pck->y);
			break;
		}
		case SC_PACKET_PUT_OBJECT:
		{
			sc_packet_put_object* put_pck = reinterpret_cast<sc_packet_put_object*>(packet);
			mScene->CreateNewObject(put_pck->id, put_pck->object_type, put_pck->name, put_pck->x, put_pck->y);
			break;
		}
		case SC_PACKET_REMOVE_OBJECT:
		{
			sc_packet_remove_object* remove_pck = reinterpret_cast<sc_packet_remove_object*>(packet);
			mScene->EraseObject(remove_pck->id);
			break;
		}
		case SC_PACKET_CHAT:
		{
			sc_packet_chat* chat_pck = reinterpret_cast<sc_packet_chat*>(packet);
			mScene->UpdatePlayerChat(chat_pck->id, chat_pck->message);
			std::wstring player_name = mScene->GetPlayerName(chat_pck->id);
			mChatWin->AppendMessage(player_name, CharToWString(chat_pck->message));
			break;
		}
		case SC_PACKET_BATTLE_RESULT:
		{
			sc_packet_battle_result* battle_pck = reinterpret_cast<sc_packet_battle_result*>(packet);
			if (battle_pck->result_type != -1) {
				std::wstring target_name = mScene->GetPlayerName(battle_pck->target);
				mLogWin->AppendLog(target_name, battle_pck->value, battle_pck->result_type);
			}
			else mScene->CreateAttackArea();
			break;
		}
		case SC_PACKET_STATUS_CHANGE:
		{
			sc_packet_status_change* status_pck = reinterpret_cast<sc_packet_status_change*>(packet);
			mScene->UpdatePlayerStatus(*status_pck);
			break;
		}
		default:
		{
			OutputDebugStringW(L"Unkown packet\n");
			Disconnect();
			return;
		}
		}
	}
}
