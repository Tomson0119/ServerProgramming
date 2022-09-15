#include "common.h"
#include "IOCPServer.h"
#include "SectorManager.h"

#include <csignal>

std::array<std::shared_ptr<Session>, MAX_USER + MAX_NPC> IOCPServer::gClients;

Timer IOCPServer::gTimer;
IOCP IOCPServer::gIOCP;


IOCPServer::IOCPServer(const EndPoint& ep)
	: mLoop(true)
{
	std::signal(SIGINT, SignalHandler);

	if (mDBHandler.ConnectToDB(L"sql_server") == false)
		std::cout << "failed to connect to DB\n";

	for (int i = 0; i < gClients.size(); i++) {
		gClients[i] = std::make_shared<Session>();
		gClients[i]->ID = i;
	}

	mSectorManager = std::make_unique<SectorManager>(SECTOR_WIDTH, SECTOR_HEIGHT);
	MemoryPoolManager<WSAOVERLAPPEDEX>::GetInstance().Init(MaxThreads * 500);

	mListenSck.Init();
	mListenSck.Bind(ep);

	InitNPC();
}

IOCPServer::~IOCPServer()
{
	for (int i = 0; i < gClients.size(); i++)
	{
		if (Helper::IsNPC(i) == false 
			&& gClients[i]->GetState() != State::FREE)
		{
			Disconnect(i);
		}
	}
}

void IOCPServer::InitNPC()
{
	for (int i = NPC_ID_START; i <= NPC_ID_END; i++)
	{
		gClients[i]->Info.x = rand() % WORLD_WIDTH;
		gClients[i]->Info.y = rand() % WORLD_HEIGHT;
		sprintf_s(gClients[i]->Info.name, "N%d", i-NPC_ID_START+1);
		gClients[i]->Type = ClientType::NPC;
		gClients[i]->InitState(State::SLEEP);
		gClients[i]->Info.level = 5;
		gClients[i]->Info.hp = 30;
		gClients[i]->Info.max_hp = 30;
		gClients[i]->AttackPower = 10;

		gClients[i]->InitLuaEngine("Script\\npc.lua");
		gClients[i]->RegisterLuaFunc("API_SendMessage", API_SendMessage);

		mSectorManager->InsertID(i, gClients[i]->Info.x, gClients[i]->Info.y);
	}
	std::cout << "Done Initializing NPC\n";
}

void IOCPServer::Run()
{
	mListenSck.Listen();
	gIOCP.RegisterDevice(mListenSck.mSocket, 0);
	std::cout << "Listening to clients...\n";

	WSAOVERLAPPEDEX acceptEx;
	mListenSck.AsyncAccept(acceptEx);

	for (int i = 0; i < MaxThreads; i++)
		mThreads.emplace_back(NetworkThreadFunc, std::ref(*this));
	gTimer.Start(this);

	for (std::thread& thrd : mThreads)
		thrd.join();
}

void IOCPServer::NetworkThreadFunc(IOCPServer& server)
{
	try {
		while (server.mLoop)
		{
			CompletionInfo info = server.gIOCP.GetCompletionInfo();

			int client_id = static_cast<int>(info.key);
			WSAOVERLAPPEDEX* over_ex = reinterpret_cast<WSAOVERLAPPEDEX*>(info.overEx);

			if (over_ex == nullptr)
			{
				server.mLoop = false;
				continue;
			}

			if (info.success == FALSE)
			{
				server.Disconnect(client_id);
				if (over_ex->Operation == OP::SEND)
					delete over_ex;
				continue;
			}

			server.HandleCompletionInfo(over_ex, client_id, info.bytes);
		}
	}
	catch (std::exception& ex)
	{
		std::cout << ex.what() << std::endl;
	}
}

void IOCPServer::HandleCompletionInfo(WSAOVERLAPPEDEX* over, int id, int bytes)
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
		over->NetBuffer.ShiftWritePtr(bytes);
		ProcessPackets(over, id, bytes);

		Session* client = gClients[id].get();
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
		SOCKET clientSck = *reinterpret_cast<SOCKET*>(over->NetBuffer.BufStartPtr());

		int new_id = GetAvailableID();
		if (new_id == -1) {
			std::cout << "Max number of clients overflow\n";
			SendLoginFailPacket(new_id, 1);
		}
		else
			AcceptNewClient(new_id, clientSck);
		mListenSck.AsyncAccept(*over);
		break;
	}

	case OP::NPC_MOVE:
	{
		if (gClients[id]->IsState(State::INGAME) == false)
		{
			delete over;
			break;
		}

		MoveNPC(id, over->Target);
		bool keep_alive = false;
		for (int i=0;i<NPC_ID_START;i++)
		{
			if (Helper::IsNear(
				gClients[id]->Info, 
				gClients[i]->Info) == false)
				continue;

			// NPC is activated only when player is near by.
			if (gClients[i]->IsState(State::INGAME))
			{
				keep_alive = true;
				break;
			}
		}
		if (keep_alive)
		{
			AddTimer(id, over->Target,
				EventType::NPC_MOVE, 1000);
		}
		else gClients[id]->InitState(State::SLEEP);
		delete over;
		break;
	}
	case OP::PLAYER_MOVE:
	{
		delete over;
		break;
	}
	}
}

void IOCPServer::MoveNPC(int id, int target)
{
	short prevx = gClients[id]->Info.x;
	short prevy = gClients[id]->Info.y;

	std::unordered_set<int> old_viewlist;
	for(int playerId = 0; playerId < MAX_USER; playerId++)
	{
		if (gClients[playerId]->IsState(State::INGAME) == false)
			continue;

		if (Helper::IsNear(
			gClients[id]->Info,
			gClients[playerId]->Info) == false)
			continue;

		old_viewlist.insert(playerId);
	}

	int direction = Helper::GetDirectionToTarget(
		gClients[id]->Info,
		gClients[target]->Info);

	Helper::MovePosition(gClients[id]->Info.x, gClients[id]->Info.y, direction);

	mSectorManager->MoveID(id, prevx, prevy, 
		gClients[id]->Info.x, gClients[id]->Info.y);

	std::unordered_set<int> new_viewlist;
	for(int playerId = 0; playerId < MAX_USER; playerId++)
	{
		if (gClients[playerId]->IsState(State::INGAME) == false)
			continue;

		if (Helper::IsNear(
			gClients[id]->Info,
			gClients[playerId]->Info) == false)
			continue;

		new_viewlist.insert(gClients[playerId]->ID);
		HandleNPCAttack(id, playerId);
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

void IOCPServer::HandleDeadNPC(int id)
{
	for(int playerId = 0; playerId < MAX_USER; playerId++)
	{
		if (!gClients[playerId]->IsState(State::INGAME))
			continue;

		if (Helper::IsNear(
			gClients[id]->Info,
			gClients[playerId]->Info) == false)
			continue;

		gClients[playerId]->EraseViewID(id);
		SendRemovePacket(playerId, id);
	}
}

void IOCPServer::ReviveNPC(int id)
{
	gClients[id]->Revive();

	int target = -1;
	for(int playerId = 0; playerId < MAX_USER; playerId++)
	{
		if (Helper::IsNear(
			gClients[id]->Info,
			gClients[playerId]->Info) == false)
			continue;

		if (!gClients[playerId]->IsState(State::INGAME))
			continue;

		gClients[playerId]->InsertViewID(id);
		SendPutObjectPacket(playerId, id);

		if (target < 0) target = playerId;
	}
	if (target >= 0)
	{
		AddTimer(id, target, EventType::NPC_MOVE, 1000);
	}
	else
	{
		gClients[id]->InitState(State::SLEEP);
	}
}

void IOCPServer::HandleNPCAttack(int npcId, int playerId)
{
	const PlayerInfo& playerInfo = gClients[playerId]->Info;
	if (gClients[npcId]->IsSamePosition(playerInfo.x, playerInfo.y) == false) return;
	if (gClients[npcId]->IsAttackTimeOut() == false) return;

	gClients[npcId]->ExecuteLuaFunc("event_npc_attack", playerId);
	gClients[npcId]->SetAttackDuration(1000ms);
	gClients[playerId]->DecreaseHP(gClients[npcId]->AttackPower);
	if (gClients[playerId]->IsDead())
	{
		gClients[playerId]->Revive();
	}
	SendBattleResultPacket(playerId, npcId, gClients[npcId]->AttackPower, 1);
	SendStatusChangePacket(playerId);
}

void IOCPServer::Disconnect(int id)
{
	std::cout << "Disconnect [" << id << "]\n";

	auto viewlist = gClients[id]->GetViewList();
	for (int pid : viewlist)
	{
		if (Helper::IsNPC(pid) 
			|| !gClients[pid]->IsState(State::INGAME))
			continue;

		if (gClients[pid]->FindAndEraseViewID(id))
			SendRemovePacket(pid, id);
	}
	if(strncmp(gClients[id]->Info.name, "GM", 2)!=0)
		mDBHandler.DisconnectAndUpdate(gClients[id]->Info);
	gClients[id]->Disconnect();
}

void IOCPServer::AcceptNewClient(int id, SOCKET sck)
{
	gClients[id]->AssignAcceptedID(id, sck);
	gIOCP.RegisterDevice(sck, id);
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

void IOCPServer::ProcessPackets(WSAOVERLAPPEDEX* over, int id, int bytes)
{
	while (over->NetBuffer.Readable())
	{
		std::byte* packet = over->NetBuffer.BufReadPtr();
		unsigned char type = static_cast<unsigned char>(GetPacketType(packet));

		if (packet == nullptr)
		{
			over->NetBuffer.Clear();
			break;
		}

		switch (type)
		{
		case CS_PACKET_LOGIN:
		{
			cs_packet_login* packet_login = reinterpret_cast<cs_packet_login*>(packet);
			ProcessLoginPacket(packet_login, id);
			break;
		}
		case CS_PACKET_TELEPORT:
		case CS_PACKET_MOVE:
		{
			short prevx = gClients[id]->Info.x;
			short prevy = gClients[id]->Info.y;

			if (type == CS_PACKET_MOVE)
			{
				cs_packet_move* packet_move = reinterpret_cast<cs_packet_move*>(packet);
				Helper::MovePosition(gClients[id]->Info.x, gClients[id]->Info.y, packet_move->direction);
				gClients[id]->LastMoveTime = packet_move->move_time;
			}
			else
			{
				cs_packet_teleport* packet_teleport = reinterpret_cast<cs_packet_teleport*>(packet);
				gClients[id]->Info.x = rand() % WORLD_WIDTH;
				gClients[id]->Info.y = rand() % WORLD_HEIGHT;
			}

			mSectorManager->MoveID(id, prevx, prevy,
				gClients[id]->Info.x, gClients[id]->Info.y);

			auto nearIds = mSectorManager->GetNearSectorIndexes(
				gClients[id]->Info.x, gClients[id]->Info.y);

			std::unordered_set<int> nearlist;
			for (const std::pair<int, int>& p : nearIds)
			{
				int row = p.first;
				int col = p.second;
			
				const auto idSet = mSectorManager->GetIDsInSector(row, col);
				for(int other : idSet)
				{
					if (gClients[other]->IsState(State::INGAME) == false
						&& gClients[other]->IsState(State::SLEEP) == false)
						continue;

					if (other == id || Helper::IsNear(
						gClients[other]->Info,
						gClients[id]->Info) == false)
						continue;

					if (Helper::IsNPC(other))
					{
						ActivateNPC(other, id);
						HandleNPCAttack(other, id);
					}

					nearlist.insert(other);
				}
			}

			SendMovePacket(id, id);

			std::unordered_set<int> viewlist = gClients[id]->GetViewList();
			HandlePlayersInSight(nearlist, viewlist, id);
			HandleDisappearedPlayers(nearlist, viewlist, id);
			break;
		}
		case CS_PACKET_CHAT:
		{
			cs_packet_chat* chat_packet = reinterpret_cast<cs_packet_chat*>(packet);
			SendChatPacket(id, id, chat_packet->message);

			auto viewlist = gClients[id]->GetViewList();
			for (int pid : viewlist)
			{
				if (Helper::IsNPC(pid) == true) continue;
				SendChatPacket(pid, id, chat_packet->message);
			}
			break;
		}
		case CS_PACKET_ATTACK:
		{
			cs_packet_attack* attack_packet = reinterpret_cast<cs_packet_attack*>(packet);
			
			if (gClients[id]->IsAttackTimeOut())
			{
				gClients[id]->SetAttackDuration(1000ms);
				auto viewlist = gClients[id]->GetViewList();
				ProcessAttackPacket(id, viewlist);
			}
			break;
		}
		default:
			std::cout << "Unkown packet\n";
			over->NetBuffer.Clear();
			return;
		}
	}
}

void IOCPServer::ProcessLoginPacket(cs_packet_login* pck, int myId)
{
	if (strncmp(pck->name, "GM", 2) == 0)
	{
		strncpy_s(gClients[myId]->Info.name, pck->name, strlen(pck->name));
		gClients[myId]->Info.exp = 100;
		gClients[myId]->Info.hp = 1000;
		gClients[myId]->Info.level = 1000;
		gClients[myId]->Info.max_hp = 10000;
		gClients[myId]->Info.x = 100;
		gClients[myId]->Info.y = 100;
		gClients[myId]->AttackPower = 1000;
	}
	else {
		auto p = mDBHandler.ConnectWithID(pck->name);
		if (p.first != 1) {
			SendLoginFailPacket(myId, p.first);
			gClients[myId]->Disconnect();
			return;
		}
		gClients[myId]->Info = p.second;
		gClients[myId]->AttackPower = gClients[myId]->Info.level * 5;
		gClients[myId]->SetAttackDuration(1000ms);
	}
	mSectorManager->InsertID(myId, gClients[myId]->Info.x, gClients[myId]->Info.y);

	if (gClients[myId]->CompareAndChangeState(State::ACCEPT, State::INGAME) == false)
	{
		std::cout << "Client is not in accept state [" << myId << "]\n";
		return; 
	}

	SendLoginOkPacket(myId);
	SendNearPlayersInfo(myId);
}

void IOCPServer::ProcessAttackPacket(int id, const std::unordered_set<int>& viewlist)
{
	int start_row = gClients[id]->Info.y - 1;
	int start_col = gClients[id]->Info.x - 1;

	for (int pid : viewlist)
	{
		if (Helper::IsNPC(pid) == false) continue;

		for (int row = start_row; row < start_row + 3; row++)
		{
			for (int col = start_col; col < start_col + 3; col++)
			{
				if (gClients[pid]->IsSamePosition(col, row)) {
					gClients[pid]->DecreaseHP(gClients[id]->AttackPower);
					SendBattleResultPacket(id, pid, gClients[id]->AttackPower, 0);

					gClients[pid]->ExecuteLuaFunc("event_npc_hurt", id);

					if (gClients[pid]->IsDead())
					{
						int val = gClients[id]->IncreaseEXP(gClients[pid]->Info.level);
						SendStatusChangePacket(id);
						SendBattleResultPacket(id, pid, val, 2);
						HandleDeadNPC(pid);
						gClients[pid]->InitState(State::DEAD);
						AddTimer(pid, id, EventType::NPC_REVIVE, 3000);
					}
				}
			}
		}
	}
	SendBattleResultPacket(id, 0, 0, -1);
}

void IOCPServer::SendNearPlayersInfo(int target)
{
	const auto nearIds = mSectorManager->GetNearSectorIndexes(
		gClients[target]->Info.x, gClients[target]->Info.y);

	for (const std::pair<int, int>& p : nearIds)
	{
		int row = p.first;
		int col = p.second;

		const auto idSet = mSectorManager->GetIDsInSector(row, col);
		for (int other : idSet)
		{
			if (gClients[other]->IsState(State::INGAME) == false
				&& gClients[other]->IsState(State::SLEEP) == false)
				continue;

			if (other == target || Helper::IsNear(
				gClients[other]->Info,
				gClients[target]->Info) == false)
				continue;

			gClients[target]->InsertViewID(other);
			SendPutObjectPacket(target, other);

			if (Helper::IsNPC(other) == false)
			{
				gClients[other]->InsertViewID(target);
				SendPutObjectPacket(other, target);
			}
			else
			{
				ActivateNPC(other, target);
				HandleNPCAttack(other, target);
			}
		}
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

		if (Helper::IsNPC(pid)) continue;

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

			if (Helper::IsNPC(pid)) continue;

			if (gClients[pid]->FindAndEraseViewID(myId))
				SendRemovePacket(pid, myId);
		}
	}
}

void IOCPServer::PostNPCMoveEvent(int objectId, int targetId)
{
	WSAOVERLAPPEDEX* over_ex = new WSAOVERLAPPEDEX(OP::NPC_MOVE);
	over_ex->Target = targetId;
	gIOCP.PostToCompletionQueue(over_ex, objectId);
}

void IOCPServer::SendLoginOkPacket(int id)
{
	sc_packet_login_ok ok_packet{};
	ok_packet.id = id;
	ok_packet.size = sizeof(sc_packet_login_ok);
	ok_packet.type = SC_PACKET_LOGIN_OK;
	ok_packet.x = gClients[id]->Info.x;
	ok_packet.y = gClients[id]->Info.y;
	ok_packet.level = gClients[id]->Info.level;
	ok_packet.hp = gClients[id]->Info.hp;
	ok_packet.maxhp = gClients[id]->Info.max_hp;
	ok_packet.exp = gClients[id]->Info.exp;
	gClients[id]->SendMsg(reinterpret_cast<std::byte*>(&ok_packet), sizeof(ok_packet));
}

void IOCPServer::SendLoginFailPacket(int id, char reason)
{
	sc_packet_login_fail fail_packet{};
	fail_packet.size = sizeof(sc_packet_login_fail);
	fail_packet.type = SC_PACKET_LOGIN_FAIL;
	fail_packet.reason = reason;
	gClients[id]->SendMsg(reinterpret_cast<std::byte*>(&fail_packet), sizeof(fail_packet));
}

void IOCPServer::SendPutObjectPacket(int sender, int target)
{
	sc_packet_put_object put_packet{};
	put_packet.id = target;
	put_packet.size = sizeof(sc_packet_put_object);
	put_packet.type = SC_PACKET_PUT_OBJECT;
	put_packet.x = gClients[target]->Info.x;
	put_packet.y = gClients[target]->Info.y;
	strcpy_s(put_packet.name, gClients[target]->Info.name);
	if (Helper::IsNPC(target))
		put_packet.object_type = 1;
	else
		put_packet.object_type = 0;
	gClients[sender]->SendMsg(reinterpret_cast<std::byte*>(&put_packet), sizeof(put_packet));
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
	gClients[sender]->SendMsg(reinterpret_cast<std::byte*>(&move_packet), sizeof(move_packet));
}

void IOCPServer::SendRemovePacket(int sender, int target)
{
	sc_packet_remove_object remove_packet{};
	remove_packet.id = target;
	remove_packet.size = sizeof(sc_packet_remove_object);
	remove_packet.type = SC_PACKET_REMOVE_OBJECT;
	gClients[sender]->SendMsg(reinterpret_cast<std::byte*>(&remove_packet), sizeof(remove_packet));
}

void IOCPServer::SendStatusChangePacket(int sender)
{
	sc_packet_status_change status_packet{};
	status_packet.size = sizeof(sc_packet_status_change);
	status_packet.type = SC_PACKET_STATUS_CHANGE;
	status_packet.level = gClients[sender]->Info.level;
	status_packet.hp = gClients[sender]->Info.hp;
	status_packet.maxhp = gClients[sender]->Info.max_hp;
	status_packet.exp = gClients[sender]->Info.exp;
	gClients[sender]->SendMsg(reinterpret_cast<std::byte*>(&status_packet), sizeof(status_packet));
}

void IOCPServer::SendBattleResultPacket(int sender, int target, int val, char type)
{
	sc_packet_battle_result result_packet{};
	result_packet.size = sizeof(sc_packet_battle_result);
	result_packet.type = SC_PACKET_BATTLE_RESULT;
	result_packet.result_type = type;
	result_packet.target = target;
	result_packet.value = val;
	gClients[sender]->SendMsg(reinterpret_cast<std::byte*>(&result_packet), sizeof(result_packet));
}

void IOCPServer::SendChatPacket(int sender, int target, char* msg)
{
	sc_packet_chat chat_packet{};
	chat_packet.id = target;
	chat_packet.size = sizeof(sc_packet_chat);
	chat_packet.type = SC_PACKET_CHAT;
	strcpy_s(chat_packet.message, msg);
	gClients[sender]->SendMsg(reinterpret_cast<std::byte*>(&chat_packet), sizeof(chat_packet));
}

void IOCPServer::AddTimer(int obj_id, int player_id, EventType type, int duration)
{
	TimerEvent ev{};
	ev.ObjectID = obj_id;
	ev.StartTime = std::chrono::system_clock::now() + std::chrono::milliseconds(duration);
	ev.EvntType = type;
	ev.TargetID = player_id;
	gTimer.AddTimerEvent(ev);
}

void IOCPServer::ActivateNPC(int npcId, int playerId)
{
	if (gClients[npcId]->CompareAndChangeState(State::SLEEP, State::INGAME))
	{
		AddTimer(npcId, playerId, EventType::NPC_MOVE, 1000);
	}
}

void IOCPServer::ActivatePlayerMoveEvent(int target, int player)
{
	WSAOVERLAPPEDEX* over_ex = new WSAOVERLAPPEDEX(OP::PLAYER_MOVE);
	over_ex->Target = player;
	gIOCP.PostToCompletionQueue(over_ex, target);
}

//int IOCPServer::API_NPCMoveTimerEvent(lua_State* ls)
//{
//	int my_id = (int)lua_tointeger(ls, -2);
//	int player = (int)lua_tointeger(ls, -1);
//	lua_pop(ls, 3);
//	AddTimer(my_id, player, EventType::NPC_MOVE, 1000);
//	return 0;
//}

int IOCPServer::API_SendMessage(lua_State* ls)
{
	int my_id = (int)lua_tointeger(ls, -3);
	int user_id = (int)lua_tointeger(ls, -2);
	char* mess = (char*)lua_tostring(ls, -1);
	lua_pop(ls, 4);
	SendChatPacket(user_id, my_id, mess);
	return 0;
}

//int IOCPServer::API_get_x(lua_State* L)
//{
//	int user_id = (int)lua_tointeger(L, -1);
//	lua_pop(L, 2);
//	int x = gClients[user_id]->Info.x;
//	lua_pushnumber(L, x);
//	return 1;
//}
//
//int IOCPServer::API_get_y(lua_State* L)
//{
//	int user_id = (int)lua_tointeger(L, -1);
//	lua_pop(L, 2);
//	int y = gClients[user_id]->Info.y;
//	lua_pushnumber(L, y);
//	return 1;
//}

void IOCPServer::SignalHandler(int signal)
{
	if (signal == SIGINT)
	{
		std::cout << "Shutting down server...\n";
		for (int i = 0; i < MaxThreads; i++)
		{
			gIOCP.PostToCompletionQueue(nullptr, -1);
		}
	}
}