#include "common.h"
#include "IOCPServer.h"


IOCPServer::IOCPServer(const EndPoint& ep)
	: mLoop(true)
{
	for (int i = 0; i < MaxPlayers; i++)
		mClients[i].ID = i;

	mListenSck.Bind(ep);
}

IOCPServer::~IOCPServer()
{
}

void IOCPServer::Run()
{
	mListenSck.Listen();
	mIOCP.RegisterDevice(mListenSck.mSocket, 0);
	std::cout << "Listening to clients...\n";

	WSAOVERLAPPEDEX acceptEx;
	mListenSck.AsyncAccept(acceptEx);

	for (int i = 0; i < MaxThreads; i++)
		mThreads.emplace_back(RunThread, std::ref(*this));

	for (std::thread& thrd : mThreads)
		thrd.join();
}

void IOCPServer::RunThread(IOCPServer& server)
{
	try {
		while (server.mLoop)
		{
			CompletionInfo info = server.mIOCP.GetCompletionInfo();
			std::cout << "GQCS returned [";

			int client_id = static_cast<int>(info.key);
			WSAOVERLAPPEDEX* over_ex = reinterpret_cast<WSAOVERLAPPEDEX*>(info.overEx);

			std::cout << (int)over_ex->Operation << "]\n";

			if (info.success == FALSE)
			{
				std::cout << "GQCS failed.. Disconnect client..\n";

				server.Disconnect(client_id);
				if (over_ex->Operation == OP::SEND)
					delete over_ex;
				continue;
			}

			switch (over_ex->Operation)
			{
			case OP::RECV:
			{
				if (info.bytes == 0)
				{
					server.Disconnect(client_id);
					continue;
				}
				Session& client = server.mClients[client_id];

				// TODO: MsgQueue에 대한 데이터레이스를 확인해야 함.
				client.MsgQueue.Push(over_ex->NetBuffer, info.bytes);
				server.ProcessPackets(client_id);
				client.RecvMsg();
				break;
			}
			case OP::SEND:
			{
				if (info.bytes != over_ex->WSABuffer.len)
					server.Disconnect(client_id);
				delete over_ex;
				break;
			}
			case OP::ACCEPT:
			{
				SOCKET clientSck = *reinterpret_cast<SOCKET*>(over_ex->NetBuffer);
				
				int new_id = server.GetAvailableID();
				if(new_id == -1)
					std::cout << "Max number of clients overflow\n";
				else
					server.AcceptNewClient(server.GetAvailableID(), clientSck);
				server.mListenSck.AsyncAccept(*over_ex);
				break;
			}
			}
		}
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
	}
}

void IOCPServer::Disconnect(int id)
{
	std::cout << "Disconnect [" << id << "]\n";
}

void IOCPServer::AcceptNewClient(int id, SOCKET sck)
{
	std::cout << "Accepted new client.\n";
	mClients[id].AssignAcceptedID(id, sck);

	mIOCP.RegisterDevice(sck, id);
	mClients[id].RecvMsg();
}

int IOCPServer::GetAvailableID()
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (mClients[i].CompareAndChangeState(State::FREE, State::ACCEPT))
			return i;
	}
	std::cout << "Maximum Number of Clients Overflow!!\n";
	return -1;
}

void IOCPServer::ProcessPackets(int id)
{
	RingBuffer& msgQue = mClients[id].MsgQueue;
	while (!msgQue.IsEmpty())
	{
		char type = msgQue.GetMsgType();

		switch (type)
		{
		case CS_PACKET_LOGIN:
		{
			cs_packet_login packet_login{};
			msgQue.Pop(reinterpret_cast<uchar*>(&packet_login), sizeof(cs_packet_login));
			
			std::cout << "Received login packet from [" << id << "]..\n";
			ProcessLoginPacket(packet_login, id);
			break;
		}
		}
	}
}

void IOCPServer::ProcessLoginPacket(cs_packet_login& pck, int myId)
{
	if (mClients[myId].CompareAndChangeState(State::ACCEPT, State::INGAME) == false)
	{
		std::cout << "Client is not in accept state [" << myId << "]\n";
		return;
	}

	
}
