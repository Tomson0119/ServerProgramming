#include "stdafx.h"
#include "clientSocket.h"

ClientSocket::ClientSocket()
	: Socket(), ID(-1), 
	  PrevSize(0), mLoop(true), 
	  Dirty(false), mIOCP{}
{
	Init();
}

ClientSocket::~ClientSocket()
{
	mSocketThread.join();
}

void ClientSocket::CreateIOCP()
{
	mIOCP.RegisterDevice(mSocket, 0);
}

void ClientSocket::AssignThread()
{
	mSocketThread = std::thread{ ClientSocket::Update, std::ref(*this) };
}

void ClientSocket::SendLoginPacket()
{
	cs_packet_login login_packet{};
	login_packet.size = sizeof(cs_packet_login);
	login_packet.type = CS_PACKET_LOGIN;
	SendMsg(reinterpret_cast<char*>(&login_packet), login_packet.size);
}

void ClientSocket::SendMovePacket(char input)
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
		dir = 1;
		break;
	case 0x27:
		dir = 3;
		break;
	case 0x28:
		dir = 0;
		break;
	}
	move_packet.direction = dir;
	std::cout << "Send Move Packet\n";
	SendMsg(reinterpret_cast<char*>(&move_packet), move_packet.size);
}

void ClientSocket::Disconnect()
{
	std::cout << "Disconnect\n";
	mLoop = false;
	cs_packet_quit pck{};
	pck.size = sizeof(cs_packet_quit);
	pck.type = CS_PACKET_QUIT;
	SendMsg(reinterpret_cast<char*>(&pck), pck.size);
}

void ClientSocket::Update(ClientSocket& client)
{
	try {
		while (client.mLoop)
		{
			CompletionInfo info = client.mIOCP.GetCompletionInfo();
			WSAOVERLAPPEDEX* over_ex = reinterpret_cast<WSAOVERLAPPEDEX*>(info.overEx);

			if (info.success == FALSE)
			{
				client.Disconnect();
				if (over_ex->Operation == OP::SEND)
					delete over_ex;
				continue;
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
		std::cout << ex.what() << std::endl;
	}
}

void ClientSocket::SendMsg(char* msg, int bytes)
{
	WSAOVERLAPPEDEX* send_over = new WSAOVERLAPPEDEX(OP::SEND, msg, bytes);
	Send(*send_over);
}

void ClientSocket::RecvMsg()
{
	mRecvOverlapped.Reset(OP::RECV);
	Recv(mRecvOverlapped);
}

void ClientSocket::ProcessPackets()
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
			ID = login_packet.id;
			PlayerInfoLock.lock();
			PlayerInfos[ID] = { login_packet.x, login_packet.y };
			PlayerInfoLock.unlock();
			Dirty = true;
			break;
		}
		case SC_PACKET_MOVE:
		{
			sc_packet_move move_pck{};
			mMsgQueue.Pop(reinterpret_cast<uchar*>(&move_pck), sizeof(sc_packet_move));
			int mover = move_pck.id;
			PlayerInfoLock.lock();
			PlayerInfos[mover] = { move_pck.x, move_pck.y };
			PlayerInfoLock.unlock();
			break;
		}
		case SC_PACKET_PUT_OBJECT:
		{
			sc_packet_put_object put_pck{};
			mMsgQueue.Pop(reinterpret_cast<uchar*>(&put_pck), sizeof(sc_packet_put_object));
			int newId = put_pck.id;
			char obj = put_pck.object_type;
			PlayerInfoLock.lock();
			PlayerInfos[newId] = { put_pck.x,put_pck.y };
			PlayerInfoLock.unlock();
			Dirty = true;
			break;
		}
		case SC_PACKET_REMOVE_OBJECT:
		{
			sc_packet_remove_object remove_pck{};
			mMsgQueue.Pop(reinterpret_cast<uchar*>(&remove_pck), sizeof(sc_packet_remove_object));
			PlayerInfoLock.lock();
			PlayerInfos.erase(remove_pck.id);
			PlayerInfoLock.unlock();
			Dirty = true;
			break;
		}

		case SC_PACKET_CHAT:
		{
			std::cout << "Chat Packet Arrived\n";
			sc_packet_chat chat_pck{};
			mMsgQueue.Pop(reinterpret_cast<uchar*>(&chat_pck), sizeof(sc_packet_chat));
			PlayerInfoLock.lock();
			strcpy_s(PlayerInfos[chat_pck.id].message, chat_pck.message);
			PlayerInfoLock.unlock();
			break;
		}
		}
	}	
}
