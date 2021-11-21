#include "stdafx.h"
#include "clientSocket.h"

ClientSocket::ClientSocket()
	: ID(-1), 
	  PrevSize(0), mLoop(true), 
	  Dirty(false), mIOCP{}
{
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
	std::cout << "thread start..\n";
	while (client.mLoop)
	{
		CompletionInfo info = client.mIOCP.GetCompletionInfo();
		std::cout << "GQCS returned [";
		WSAOVERLAPPEDEX* over_ex = reinterpret_cast<WSAOVERLAPPEDEX*>(info.overEx);
		std::cout << (int)over_ex->Operation << "]\n";

		if (info.success == FALSE)
		{
			std::cout << "GQCS failed.. Disconnect client..\n";

			client.Disconnect();
			if (over_ex->Operation == OP::SEND)
				delete over_ex;
			continue;
		}

		switch (over_ex->Operation)
		{
		case OP::RECV:
			break;

		case OP::SEND:
			if (info.bytes != over_ex->WSABuffer.len)
				client.Disconnect();
			delete over_ex;
			break;
		}
	}
}

void ClientSocket::SendMsg(char* msg, int bytes)
{
	WSAOVERLAPPEDEX* send_over = new WSAOVERLAPPEDEX(OP::SEND, msg, bytes);
	Send(*send_over);
}

void ClientSocket::RecvMsg()
{
	/*mRecvOverlapped.Reset(OP::RECV, PrevSize);
	Recv(&mRecvOverlapped);*/
}

void ClientSocket::HandleMessage(unsigned char* msg)
{
	unsigned char type = msg[1];
	switch (type)
	{
	case SC_PACKET_LOGIN_OK:
	{
		sc_packet_login_ok* login_packet = reinterpret_cast<sc_packet_login_ok*>(msg);
		ID = (int)login_packet->id;
		PlayerCoords[ID] = { 0, 0 };
		Dirty = true;
		break;
	}
	case SC_PACKET_MOVE:
	{
		sc_packet_move* move_pck = reinterpret_cast<sc_packet_move*>(msg);
		int mover = (int)move_pck->id;
		short x = move_pck->x;
		short y = move_pck->y;
		PlayerCoords[mover] = { x,y };
		break;
	}
	case SC_PACKET_PUT_OBJECT:
	{
		sc_packet_put_object* put_pck = reinterpret_cast<sc_packet_put_object*>(msg);
		int newId = (int)put_pck->id;
		char obj = put_pck->object_type;
		short x = put_pck->x;
		short y = put_pck->y;
		PlayerCoords[newId] = { x,y };
		Dirty = true;
		break;
	}
	case SC_PACKET_REMOVE_OBJECT:
	{
		sc_packet_remove_object* remove_pck = reinterpret_cast<sc_packet_remove_object*>(msg);
		int target = (int)remove_pck->id;
		PlayerCoords.erase(target);
		Dirty = true;
		break;
	}
	}
}
