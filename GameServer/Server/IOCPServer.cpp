#include "common.h"
#include "IOCPServer.h"

std::array<std::shared_ptr<Session>, MAX_USER + MAX_NPC> IOCPServer::gClients;
//std::array<std::array<std::unordered_set<int>, SECTOR_WIDTH>, SECTOR_HEIGHT> IOCPServer::gSectors;
concurrency::concurrent_priority_queue<TimerEvent> IOCPServer::gTimerQueue;

IOCPServer::IOCPServer(const EndPoint& ep)
	: mLoop(true)
{
	for (int i = 0; i < gClients.size(); i++) {
		gClients[i] = std::make_shared<Session>();
		gClients[i]->ID = i;
	}

	mListenSck.Init();
	mListenSck.Bind(ep);
}

IOCPServer::~IOCPServer()
{
}

void IOCPServer::InitNPC()
{
	for (int i = NPC_ID_START; i <= NPC_ID_END; i++)
	{
		gClients[i]->Info.x = rand() % WORLD_WIDTH;
		gClients[i]->Info.y = rand() % WORLD_HEIGHT;
		sprintf_s(gClients[i]->Name, "N%d", i-NPC_ID_START+1);
		gClients[i]->Type = ClientType::NPC;
		gClients[i]->InitState(State::SLEEP);

		/*lua_State* ls = luaL_newstate();
		gClients[i]->Lua = ls;
		luaL_openlibs(ls);
		luaL_loadfile(ls, "Script\\npc.lua");
		lua_pcall(ls, 0, 0, 0);

		lua_getglobal(ls, "set_uid");
		lua_pushnumber(ls, i);
		lua_pcall(ls, 1, 0, 0);
		lua_pop(ls, 1);

		lua_register(ls, "API_AddTimer", API_AddTimer);
		lua_register(ls, "API_SendMessage", API_SendMessage);
		lua_register(ls, "API_get_x", API_get_x);
		lua_register(ls, "API_get_y", API_get_y);*/
	}
	std::cout << "Done Initializing NPC\n";
}

void IOCPServer::Run()
{
	mListenSck.Listen();
	mIOCP.RegisterDevice(mListenSck.mSocket, 0);
	std::cout << "Listening to clients...\n";

	WSAOVERLAPPEDEX acceptEx;
	mListenSck.AsyncAccept(acceptEx);

	InitNPC();

	for (int i = 0; i < MaxThreads; i++)
		mThreads.emplace_back(NetworkThreadFunc, std::ref(*this));
	mTimerThread = std::thread{ TimerThreadFunc, std::ref(*this) };

	for (std::thread& thrd : mThreads)
		thrd.join();
	mTimerThread.join();
}

void IOCPServer::NetworkThreadFunc(IOCPServer& server)
{
	try {
		while (server.mLoop)
		{
			CompletionInfo info = server.mIOCP.GetCompletionInfo();

			int client_id = static_cast<int>(info.key);
			WSAOVERLAPPEDEX* over_ex = reinterpret_cast<WSAOVERLAPPEDEX*>(info.overEx);

			if (info.success == FALSE)
			{
				server.Disconnect(client_id);
				if (over_ex->Operation == OP::SEND)
					delete over_ex;
				continue;
			}

			server.HandleCompletionInfoByOperation(over_ex, client_id, info.bytes);
		}
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
	}
}

void IOCPServer::HandleCompletionInfoByOperation(WSAOVERLAPPEDEX* over, int id, int bytes)
{
	switch (over->Operation)
	{
	case OP::RECV:
	{
		if (bytes == 0)
		{
			Disconnect(id);
			break;
		}
		Session* client = gClients[id].get();
		//std::cout << "Received Msg [" << id << "]\n";
		over->MsgQueue.Push(over->NetBuffer, bytes);
		ProcessPackets(id, over->MsgQueue);
		client->RecvMsg();
		break;
	}
	case OP::SEND:
	{
		if (bytes != over->WSABuffer.len)
			Disconnect(id);
		delete over;
		break;
	}
	case OP::ACCEPT:
	{
		SOCKET clientSck = *reinterpret_cast<SOCKET*>(over->NetBuffer);

		int new_id = GetAvailableID();
		if (new_id == -1)
			std::cout << "Max number of clients overflow\n";
		else
			AcceptNewClient(new_id, clientSck);
		mListenSck.AsyncAccept(*over);
		break;
	}

	case OP::NPC_MOVE:
	{
		MoveNPC(id, over->Random_direction);
		lua_State* ls = gClients[id]->Lua;
		lua_getglobal(ls, "event_npc_move");
		lua_pushnumber(ls, over->Target);
		lua_pushnumber(ls, 3);
		lua_pushnumber(ls, over->Random_direction);
		lua_pushstring(ls, "Bye");
		lua_pcall(ls, 4, 0, 0);		

		/*MoveNPC(id, over->Random_direction);
		bool keep_alive = false;
		for (int i=0;i<NPC_ID_START;i++)
		{
			if (IsNear(id, gClients[i]->ID) == false)
				continue;

			if (gClients[i]->IsStateWithoutLock(State::INGAME))
			{
				keep_alive = true;
				break;
			}
		}
		if (keep_alive) AddTimer(id, over->Target, EventType::NPC_MOVE, rand()%4, 1000);
		else gClients[id]->InitState(State::SLEEP);*/
		delete over;
		break;
	}
	case OP::PLAYER_MOVE:
	{
		lua_State* ls = gClients[id]->Lua;
		lua_getglobal(ls, "event_player_move");
		lua_pushnumber(ls, over->Target);
		lua_pushnumber(ls, rand() % 4);
		lua_pushstring(ls, "Hello");
		lua_pcall(ls, 3, 0, 0);
		delete over;
		break;
	}
	}
}

void IOCPServer::MoveNPC(int id, int direction)
{
	std::unordered_set<int> old_viewlist;
	std::unordered_set<int> new_viewlist;

	for (int i=0;i<NPC_ID_START;i++)
	{
		if (IsNear(id, gClients[i]->ID) == false)
			continue;
		if (!gClients[i]->IsStateWithoutLock(State::INGAME))
			continue;
		old_viewlist.insert(gClients[i]->ID);
	}

	MovePosition(gClients[id]->Info, direction);

	for (int i = 0; i < NPC_ID_START; i++)
	{
		if (IsNear(id, gClients[i]->ID) == false)
			continue;
		if (!gClients[i]->IsStateWithoutLock(State::INGAME))
			continue;
		new_viewlist.insert(gClients[i]->ID);
	}

	// For players in sight of npc
	for (auto& pl : new_viewlist)
	{
		if (old_viewlist.find(pl) == old_viewlist.end())
		{
			gClients[pl]->InsertViewID(id);
			SendPutObjectPacket(pl, id);
		}
		else
		{
			SendMovePacket(pl, id);
		}
	}

	// For players out of sights
	for (auto& pl : old_viewlist)
	{
		if (new_viewlist.find(pl) == new_viewlist.end())
		{
			gClients[pl]->EraseViewID(id);
			SendRemovePacket(pl, id);
		}
	}
}

void IOCPServer::TimerThreadFunc(IOCPServer& server)
{
	try {
		while (server.mLoop)
		{
			while (gTimerQueue.empty() == false)
			{
				TimerEvent evnt;
				gTimerQueue.try_pop(evnt);
				if (evnt.StartTime <= std::chrono::system_clock::now())
				{
					switch (evnt.EvntType)
					{
					case EventType::NPC_MOVE:
					{
						WSAOVERLAPPEDEX* over_ex = new WSAOVERLAPPEDEX(OP::NPC_MOVE);
						over_ex->Target = evnt.TargetID;
						over_ex->Random_direction = evnt.Move_direction;
						server.mIOCP.PostToCompletionQueue(over_ex, evnt.ObjectID);
						break;
					}
					}
				}
				else
				{
					gTimerQueue.push(evnt);
					break;
				}
			}
			std::this_thread::sleep_for(10ms);
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
	
	gClients[id]->ViewListLock.lock();
	auto viewlist = gClients[id]->GetViewList();
	gClients[id]->ViewListLock.unlock();

	for (int pid : viewlist)
	{
		if (IsNPC(pid) || !gClients[pid]->IsStateWithoutLock(State::INGAME))
			continue;

		if (gClients[pid]->FindAndEraseViewID(id))
			SendRemovePacket(pid, id);
	}
	gClients[id]->Disconnect();
}

void IOCPServer::AcceptNewClient(int id, SOCKET sck)
{
	gClients[id]->AssignAcceptedID(id, sck);
	gClients[id]->Info.x = rand() % WORLD_WIDTH;
	gClients[id]->Info.y = rand() % WORLD_HEIGHT;

	mIOCP.RegisterDevice(sck, id);
	gClients[id]->RecvMsg();
}

int IOCPServer::GetAvailableID()
{
	for (int i = 0; i < MAX_USER; ++i)
	{
		if (gClients[i]->CompareAndChangeState(State::FREE, State::ACCEPT))
			return i;
	}
	std::cout << "Maximum Number of Clients Overflow!!\n";
	return -1;
}

void IOCPServer::ProcessPackets(int id, RingBuffer& msgQueue)
{
	while (!msgQueue.IsEmpty())
	{
		char type = msgQueue.GetMsgType();

		switch (type)
		{
		case CS_PACKET_LOGIN:
		{
			cs_packet_login packet_login{};
			msgQueue.Pop(reinterpret_cast<uchar*>(&packet_login), sizeof(cs_packet_login));
			ProcessLoginPacket(packet_login, id);
			break;
		}
		case CS_PACKET_MOVE:
		{
			cs_packet_move packet_move{};
			msgQueue.Pop(reinterpret_cast<uchar*>(&packet_move), sizeof(cs_packet_move));
			MovePosition(gClients[id]->Info, packet_move.direction);
			gClients[id]->LastMoveTime = packet_move.move_time;

			std::unordered_set<int> nearlist;			
			for (auto& other : gClients)
			{
				if (other->ID == id || IsNear(other->ID, id) == false)
					continue;
				if (other->IsStateWithoutLock(State::SLEEP))
					ActivateNPC(other->ID);
				if (!other->IsStateWithoutLock(State::INGAME))
					continue;
				if (IsNPC(other->ID))
					ActivatePlayerMoveEvent(other->ID, id);
				nearlist.insert(other->ID);
			}
			SendMovePacket(id, id);

			gClients[id]->ViewListLock.lock();
			std::unordered_set<int> viewlist = gClients[id]->GetViewList();
			gClients[id]->ViewListLock.unlock();

			HandlePlayersInSight(nearlist, viewlist, id);
			HandleDisappearedPlayers(nearlist, viewlist, id);
			break;
		}
		case CS_PACKET_QUIT:
		{
			cs_packet_quit quit_packet{};
			msgQueue.Pop(reinterpret_cast<uchar*>(&quit_packet), sizeof(cs_packet_quit));
			Disconnect(id);
			break;
		}
		default:
			std::cout << "Unkown packet\n";
			return;
		}
	}
}

void IOCPServer::ProcessLoginPacket(cs_packet_login& pck, int myId)
{
	if (gClients[myId]->CompareAndChangeState(State::ACCEPT, State::INGAME) == false)
	{
		std::cout << "Client is not in accept state [" << myId << "]\n";
		return;
	}
	strcpy_s(gClients[myId]->Name, pck.name);

	SendLoginOkPacket(myId);
	SendNewPlayerInfoToNearPlayers(myId);
	SendNearPlayersInfoToNewPlayer(myId);
}

void IOCPServer::SendNewPlayerInfoToNearPlayers(int target)
{
	for (auto& other : gClients)
	{
		if (other->ID == target || !IsNear(other->ID, target))
			continue;
		if (other->IsStateWithoutLock(State::SLEEP))
			ActivateNPC(other->ID);
		if (IsNPC(other->ID) || !other->IsStateWithoutLock(State::INGAME))
			continue;

		other->InsertViewID(target);
		SendPutObjectPacket(other->ID, target);
	}
}

void IOCPServer::SendNearPlayersInfoToNewPlayer(int sender)
{
	for (auto& client : gClients)
	{
		if (client->ID == sender || !IsNear(client->ID, sender))
			continue;
		if (!client->IsStateWithoutLock(State::INGAME))
			continue;

		gClients[sender]->InsertViewID(client->ID);
		SendPutObjectPacket(sender, client->ID);
	}
}

void IOCPServer::HandlePlayersInSight(
	const std::unordered_set<int>& sights,
	const std::unordered_set<int>& viewlist, int myId)
{	
	for (int pid : sights)
	{
		// Handle players not in my view list.
		if (viewlist.find(pid) == viewlist.end())
		{
			gClients[myId]->InsertViewID(pid);
			SendPutObjectPacket(myId, pid);
		}

		if (IsNPC(pid)) continue;

		if (gClients[pid]->FindAndInsertViewID(myId))
			SendPutObjectPacket(pid, myId);
		else
			SendMovePacket(pid, myId);
	}
}

void IOCPServer::HandleDisappearedPlayers(
	const std::unordered_set<int>& sights, 
	const std::unordered_set<int>& viewlist, int myId)
{
	for (int pid : viewlist)
	{
		if (sights.find(pid) == sights.end())
		{
			gClients[myId]->EraseViewID(pid);
			SendRemovePacket(myId, pid);

			if (IsNPC(pid)) continue;

			if (gClients[pid]->FindAndEraseViewID(myId))
				SendRemovePacket(pid, myId);
		}
	}
}

void IOCPServer::SendLoginOkPacket(int id)
{
	sc_packet_login_ok ok_packet{};
	ok_packet.id = id;
	ok_packet.size = sizeof(sc_packet_login_ok);
	ok_packet.type = SC_PACKET_LOGIN_OK;
	ok_packet.x = gClients[id]->Info.x;
	ok_packet.y = gClients[id]->Info.y;
	gClients[id]->SendMsg(reinterpret_cast<char*>(&ok_packet), sizeof(ok_packet));
}

void IOCPServer::SendPutObjectPacket(int sender, int target)
{
	sc_packet_put_object put_packet{};
	put_packet.id = target;
	put_packet.size = sizeof(sc_packet_put_object);
	put_packet.type = SC_PACKET_PUT_OBJECT;
	put_packet.x = gClients[target]->Info.x;
	put_packet.y = gClients[target]->Info.y;
	strcpy_s(put_packet.name, gClients[target]->Name);
	put_packet.object_type = 0;
	gClients[sender]->SendMsg(reinterpret_cast<char*>(&put_packet), sizeof(put_packet));
}

void IOCPServer::SendMovePacket(int sender, int target)
{
	sc_packet_move move_packet{};
	move_packet.id = target;
	move_packet.size = sizeof(sc_packet_move);
	move_packet.type = SC_PACKET_MOVE;
	move_packet.x = gClients[target]->Info.x;
	move_packet.y = gClients[target]->Info.y;
	move_packet.move_time = gClients[target]->LastMoveTime;
	gClients[sender]->SendMsg(reinterpret_cast<char*>(&move_packet), sizeof(move_packet));
}

void IOCPServer::SendRemovePacket(int sender, int target)
{
	sc_packet_remove_object remove_packet{};
	remove_packet.id = target;
	remove_packet.size = sizeof(sc_packet_remove_object);
	remove_packet.type = SC_PACKET_REMOVE_OBJECT;
	gClients[sender]->SendMsg(reinterpret_cast<char*>(&remove_packet), sizeof(remove_packet));
}

void IOCPServer::SendChatPacket(int sender, int target, char* msg)
{
	sc_packet_chat chat_packet;
	chat_packet.id = target;
	chat_packet.size = sizeof(sc_packet_chat);
	chat_packet.type = SC_PACKET_CHAT;
	strcpy_s(chat_packet.message, msg);
	gClients[sender]->SendMsg(reinterpret_cast<char*>(&chat_packet), sizeof(chat_packet));
}

bool IOCPServer::IsNPC(int id)
{
	return (id >= NPC_ID_START && id <= NPC_ID_END);
}

bool IOCPServer::IsNear(int a_id, int b_id)
{
	if (abs(gClients[a_id]->Info.x - gClients[b_id]->Info.x) > RANGE) return false;
	if (abs(gClients[a_id]->Info.y - gClients[b_id]->Info.y) > RANGE) return false;
	return true;
}

void IOCPServer::MovePosition(PlayerInfo& pos, char direction)
{
	switch (direction)
	{
	case 0: if (pos.y > 0) pos.y--; break;
	case 1: if (pos.y < WORLD_HEIGHT - 1) pos.y++; break;
	case 2: if (pos.x > 0) pos.x--; break;
	case 3: if (pos.x < WORLD_WIDTH - 1) pos.x++; break;
	}
}

void IOCPServer::AddTimer(int obj_id, int player_id, EventType type, int direction, int duration)
{
	TimerEvent ev{};
	ev.ObjectID = obj_id;
	ev.StartTime = std::chrono::system_clock::now() + std::chrono::milliseconds(duration);
	ev.EvntType = type;
	ev.TargetID = player_id;
	ev.Move_direction = direction;
	gTimerQueue.push(ev);
}

void IOCPServer::ActivateNPC(int id)
{
	if (gClients[id]->CompareAndChangeState(State::SLEEP, State::INGAME));
		//AddTimer(id, 0, EventType::NPC_MOVE, rand()%4, 1000);
}

void IOCPServer::ActivatePlayerMoveEvent(int target, int player)
{
	WSAOVERLAPPEDEX* over_ex = new WSAOVERLAPPEDEX(OP::PLAYER_MOVE);
	over_ex->Target = player;
	mIOCP.PostToCompletionQueue(over_ex, target);
}

std::pair<short, short> IOCPServer::GetSectorIndex(int id)
{
	short x = gClients[id]->Info.x;
	short y = gClients[id]->Info.y;
	return { x / SECTOR_WIDTH, y / SECTOR_HEIGHT };
}

void IOCPServer::InsertIntoSectorWithLock(int id)
{
	/*auto sector = GetSectorIndex(id);

	mSectorLock.lock();
	gSectors[sector.first][sector.second].insert(id);
	mSectorLock.unlock();*/
}

void IOCPServer::EraseFromSectorWidthLock(int id)
{
	auto sector = GetSectorIndex(id);

	/*mSectorLock.lock();
	gSectors[sector.first][sector.second].erase(id);
	mSectorLock.unlock();*/
}

int IOCPServer::API_AddTimer(lua_State* ls)
{
	int my_id = (int)lua_tointeger(ls, -3);
	int player = (int)lua_tointeger(ls, -2);
	int dir = (int)lua_tointeger(ls, -1);
	lua_pop(ls, 4);
	AddTimer(my_id, player, EventType::NPC_MOVE, dir, 1000);
	return 0;
}

int IOCPServer::API_SendMessage(lua_State* ls)
{
	int my_id = (int)lua_tointeger(ls, -3);
	int user_id = (int)lua_tointeger(ls, -2);
	char* mess = (char*)lua_tostring(ls, -1);
	lua_pop(ls, 4);
	SendChatPacket(user_id, my_id, mess);
	return 0;
}

int IOCPServer::API_get_x(lua_State* L)
{
	int user_id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int x = gClients[user_id]->Info.x;
	lua_pushnumber(L, x);
	return 1;
}

int IOCPServer::API_get_y(lua_State* L)
{
	int user_id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int y = gClients[user_id]->Info.y;
	lua_pushnumber(L, y);
	return 1;
}