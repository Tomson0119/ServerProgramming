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
	mSocketThread = std::thread{ NetClient::Update, std::ref(*this) };
	RecvMsg();
	SendLoginPacket(name.c_str());
}

void NetClient::SendLoginPacket(const char* name)
{
	cs_packet_login login_packet{};
	login_packet.size = sizeof(cs_packet_login);
	login_packet.type = CS_PACKET_LOGIN;
	strncpy_s(login_packet.name, name, strlen(name));
	SendMsg(reinterpret_cast<char*>(&login_packet), login_packet.size);
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
	SendMsg(reinterpret_cast<char*>(&move_packet), move_packet.size);
}

void NetClient::SendChatPacket(const char* msg)
{
	cs_packet_chat chat_packet{};
	chat_packet.size = sizeof(cs_packet_chat);
	chat_packet.type = CS_PACKET_CHAT;
	strncpy_s(chat_packet.message, msg, strlen(msg));
	SendMsg(reinterpret_cast<char*>(&chat_packet), chat_packet.size);
}

void NetClient::Disconnect()
{
	mLoop = false;
	mIOCP.PostToCompletionQueue(nullptr, 0);
}

void NetClient::Update(NetClient& client)
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

			switch (over_ex->Operation)
			{
			case OP::RECV:
			{
				client.mMsgQueue.Push(over_ex->NetBuffer, info.bytes);
				client.ProcessPackets();
				client.RecvMsg();
				break;
			}
			case OP::SEND:
				if (info.bytes != over_ex->WSABuffer.len)
					client.Disconnect();
				delete over_ex;
				break;
			}
		}
	}
	catch (std::exception& ex)
	{
		MessageBoxA(NULL, ex.what(), "Error", MB_OK);
	}
	CoUninitialize();
}

void NetClient::SendMsg(char* msg, int bytes)
{
	WSAOVERLAPPEDEX* send_over = new WSAOVERLAPPEDEX(OP::SEND, msg, bytes);
	Send(*send_over);
}

void NetClient::RecvMsg()
{
	mRecvOverlapped.Reset(OP::RECV);
	Recv(mRecvOverlapped);
}

void NetClient::ProcessPackets()
{
	while (!mMsgQueue.IsEmpty())
	{
		char type = mMsgQueue.GetMsgType();
		switch (type)
		{
		case SC_PACKET_LOGIN_OK:
		{
			sc_packet_login_ok login_packet{};
			mMsgQueue.Pop(reinterpret_cast<uchar*>(&login_packet), sizeof(sc_packet_login_ok));
			mScene->InitializePlayer(login_packet, mUsername.c_str());
			break;
		}
		case SC_PACKET_LOGIN_FAIL:
		{
			sc_packet_login_fail fail_packet{};
			mMsgQueue.Pop(reinterpret_cast<uchar*>(&fail_packet), sizeof(sc_packet_login_fail));
			std::wstring str = L"Login failed: ";
			
			switch (fail_packet.reason)
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
			sc_packet_move move_pck{};
			mMsgQueue.Pop(reinterpret_cast<uchar*>(&move_pck), sizeof(sc_packet_move));
			mScene->UpdatePlayerPosition(move_pck.id, move_pck.x, move_pck.y);
			break;
		}
		case SC_PACKET_PUT_OBJECT:
		{
			sc_packet_put_object put_pck{};
			mMsgQueue.Pop(reinterpret_cast<uchar*>(&put_pck), sizeof(sc_packet_put_object));
			mScene->CreateNewObject(put_pck.id, put_pck.object_type, put_pck.name, put_pck.x, put_pck.y);
			break;
		}
		case SC_PACKET_REMOVE_OBJECT:
		{
			sc_packet_remove_object remove_pck{};
			mMsgQueue.Pop(reinterpret_cast<uchar*>(&remove_pck), sizeof(sc_packet_remove_object));
			mScene->EraseObject(remove_pck.id);
			break;
		}
		case SC_PACKET_CHAT:
		{
			sc_packet_chat chat_pck{};
			mMsgQueue.Pop(reinterpret_cast<uchar*>(&chat_pck), sizeof(sc_packet_chat));
			mScene->UpdatePlayerChat(chat_pck.id, chat_pck.message);
			mChatWin->AppendMessage(chat_pck.id, chat_pck.message);
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
