#include "common.h"
#include "IOCPServer.h"

#include <csignal>

std::array<std::shared_ptr<Session>, MAX_USER + MAX_NPC> IOCPServer::gClients;
std::array<std::array<std::unordered_set<int>, SECTOR_WIDTH>, SECTOR_HEIGHT> IOCPServer::gSectors;

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

	mListenSck.Init();
	mListenSck.Bind(ep);

	InitNPC();
}

IOCPServer::~IOCPServer()
{
	for (int i = 0; i < gClients.size(); i++)
	{
		if (IsNPC(i) == false && gClients[i]->GetState() != State::FREE)
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
		gClients[i]->InitState(State::INGAME);
		gClients[i]->Info.level = 5;
		gClients[i]->Info.hp = 30;
		gClients[i]->Info.max_hp = 30;
		gClients[i]->AttackPower = 20;
		InsertIntoSectorWithoutLock(i);
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

	MovePosition(gClients[id]->Info.x, gClients[id]->Info.y, direction);

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

void IOCPServer::HandleDeadNPC(int id)
{
	for (int i = 0; i < NPC_ID_START; i++)
	{
		if (IsNear(id, gClients[i]->ID) == false)
			continue;
		if (!gClients[i]->IsStateWithoutLock(State::INGAME))
			continue;
		gClients[i]->EraseViewID(id);
		SendRemovePacket(i, id);
	}
}

void IOCPServer::HandleRevivedPlayer(int id)
{
	gClients[id]->Revive();
	for (int i = 0; i < NPC_ID_START; i++)
	{
		if (IsNear(id, gClients[i]->ID) == false)
			continue;
		if (!gClients[i]->IsStateWithoutLock(State::INGAME))
			continue;
		gClients[i]->InsertViewID(id);
		SendPutObjectPacket(i, id);
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
			auto beforeSecIdx = GetSectorIndex(id);
			
			if (type == CS_PACKET_MOVE)
			{
				cs_packet_move* packet_move = reinterpret_cast<cs_packet_move*>(packet);
				MovePosition(gClients[id]->Info.x, gClients[id]->Info.y, packet_move->direction);
				gClients[id]->LastMoveTime = packet_move->move_time;
			}
			else
			{
				cs_packet_teleport* packet_teleport = reinterpret_cast<cs_packet_teleport*>(packet);
				gClients[id]->Info.x = rand() % WORLD_WIDTH;
				gClients[id]->Info.y = rand() % WORLD_HEIGHT;
			}
			auto afterSecIdx = GetSectorIndex(id);

			if (beforeSecIdx != afterSecIdx)
			{
				mSectorLock.lock();
				gSectors[beforeSecIdx.first][beforeSecIdx.second].erase(id);
				gSectors[afterSecIdx.first][afterSecIdx.second].insert(id);
				mSectorLock.unlock();
			}

			std::unordered_set<int> nearlist;

			auto sectorIdx = GetSectorIndex(id);
			for (int row = sectorIdx.first - 1; row < sectorIdx.first + 2; row++)
			{
				if (row < 0 || row > SECTOR_HEIGHT - 1) continue;
				for (int col = sectorIdx.second - 1; col < sectorIdx.second + 2; col++)
				{
					if (col < 0 || col > SECTOR_WIDTH - 1) continue;

					mSectorLock.lock();					
					for (int cid : gSectors[row][col])
					{
						if (!gClients[cid]->IsStateWithoutLock(State::INGAME))
							continue;
						if (cid == id || IsNear(cid, id) == false)
							continue;
						nearlist.insert(cid);
					}
					mSectorLock.unlock();
				}
			}
			SendMovePacket(id, id);			

			gClients[id]->ViewListLock.lock();
			std::unordered_set<int> viewlist = gClients[id]->GetViewList();
			gClients[id]->ViewListLock.unlock();

			HandlePlayersInSight(nearlist, viewlist, id);
			HandleDisappearedPlayers(nearlist, viewlist, id);
			break;
		}
		case CS_PACKET_CHAT:
		{
			cs_packet_chat* chat_packet = reinterpret_cast<cs_packet_chat*>(packet);

			gClients[id]->ViewListLock.lock();
			auto viewlist = gClients[id]->GetViewList();
			gClients[id]->ViewListLock.unlock();

			SendChatPacket(id, id, chat_packet->message);
			for (int pid : viewlist)
			{
				if (IsNPC(pid) == true) continue;
				SendChatPacket(pid, id, chat_packet->message);
			}
			break;
		}
		case CS_PACKET_ATTACK:
		{
			cs_packet_attack* attack_packet = reinterpret_cast<cs_packet_attack*>(packet);
			
			if (gClients[id]->IsAttackTimeOut() == false) break;

			gClients[id]->ViewListLock.lock();
			auto viewlist = gClients[id]->GetViewList();
			gClients[id]->ViewListLock.unlock();

			gClients[id]->SetAttackDuration(1000ms);
			ProcessAttackPacket(id, viewlist);
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
	InsertIntoSectorWithLock(myId);	

	if (gClients[myId]->CompareAndChangeState(State::ACCEPT, State::INGAME) == false)
	{
		std::cout << "Client is not in accept state [" << myId << "]\n";
		return; 
	}

	SendLoginOkPacket(myId);
	SendNewPlayerInfoToNearPlayers(myId);
	SendNearPlayersInfoToNewPlayer(myId);
}

void IOCPServer::ProcessAttackPacket(int id, const std::unordered_set<int>& viewlist)
{
	int start_row = gClients[id]->Info.y - 1;
	int start_col = gClients[id]->Info.x - 1;

	for (int pid : viewlist)
	{
		if (IsNPC(pid) == false) continue;

		for (int row = start_row; row < start_row + 3; row++)
		{
			for (int col = start_col; col < start_col + 3; col++)
			{
				if (gClients[pid]->IsSame(col, row)) {
					gClients[pid]->DecreaseHP(gClients[id]->AttackPower);
					SendBattleResultPacket(id, pid, gClients[id]->AttackPower, 0);

					if (gClients[pid]->IsDead())
					{
						int val = gClients[id]->IncreaseEXP(gClients[pid]->Info.level);
						SendStatusChangePacket(id);
						SendBattleResultPacket(id, pid, val, 2);
						HandleDeadNPC(pid);
						gClients[pid]->InitState(State::SLEEP);
						AddTimer(pid, id, EventType::NPC_REVIVE, -1, 3000);
					}
				}
			}
		}
	}
	SendBattleResultPacket(id, 0, 0, -1);
}

void IOCPServer::SendNewPlayerInfoToNearPlayers(int target)
{
	auto sectorIdx = GetSectorIndex(target);
	for (int row = sectorIdx.first - 1; row < sectorIdx.first + 2; row++)
	{
		if (row < 0 || row > SECTOR_HEIGHT - 1)	continue;
		for (int col = sectorIdx.second - 1; col < sectorIdx.second + 2; col++)
		{
			if (col < 0 || col > SECTOR_WIDTH - 1) continue;

			mSectorLock.lock();			
			for (int cid : gSectors[row][col])
			{
				if (IsNPC(cid) || !gClients[cid]->IsStateWithoutLock(State::INGAME))
					continue;
				if (cid == target || !IsNear(cid, target))
					continue;

				gClients[cid]->InsertViewID(target);
				SendPutObjectPacket(cid, target);
			}
			mSectorLock.unlock();
		}
	}
}

void IOCPServer::SendNearPlayersInfoToNewPlayer(int sender)
{
	auto sectorIdx = GetSectorIndex(sender);
	for (int row = sectorIdx.first - 1; row < sectorIdx.first + 2; row++)
	{
		if (row < 0 || row > SECTOR_HEIGHT - 1)	continue;
		for (int col = sectorIdx.second - 1; col < sectorIdx.second + 2; col++)
		{
			if (col < 0 || col > SECTOR_WIDTH - 1) continue;

			mSectorLock.lock();			
			for (int cid : gSectors[row][col])
			{
				if (!gClients[cid]->IsStateWithoutLock(State::INGAME))
					continue;
				if (cid == sender || !IsNear(cid, sender))
					continue;

				gClients[sender]->InsertViewID(cid);
				SendPutObjectPacket(sender, cid);
			}
			mSectorLock.unlock();
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

void IOCPServer::PostNPCMoveEvent(int objectId, int targetId, int direction)
{
	WSAOVERLAPPEDEX* over_ex = new WSAOVERLAPPEDEX(OP::NPC_MOVE);
	over_ex->Target = targetId;
	over_ex->Random_direction = direction;
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
	if (IsNPC(target))
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
	sc_packet_chat chat_packet;
	chat_packet.id = target;
	chat_packet.size = sizeof(sc_packet_chat);
	chat_packet.type = SC_PACKET_CHAT;
	strcpy_s(chat_packet.message, msg);
	gClients[sender]->SendMsg(reinterpret_cast<std::byte*>(&chat_packet), sizeof(chat_packet));
}

bool IOCPServer::IsNPC(int id)
{
	return (id >= NPC_ID_START && id <= NPC_ID_END);
}

bool IOCPServer::IsNear(int a_id, int b_id)
{
	if (ABS(gClients[a_id]->Info.x - gClients[b_id]->Info.x) > RANGE) return false;
	if (ABS(gClients[a_id]->Info.y - gClients[b_id]->Info.y) > RANGE) return false;
	return true;
}

void IOCPServer::MovePosition(short& x, short& y, char direction)
{
	switch (direction)
	{
	case 0: if (y > 0) y--; break;
	case 1: if (y < WORLD_HEIGHT - 1) y++; break;
	case 2: if (x > 0) x--; break;
	case 3: if (x < WORLD_WIDTH - 1) x++; break;
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
	gTimer.AddTimerEvent(ev);
}

void IOCPServer::ActivateNPC(int id)
{
	if (gClients[id]->CompareAndChangeState(State::SLEEP, State::INGAME))
		AddTimer(id, 0, EventType::NPC_MOVE, rand()%4, 1000);
}

void IOCPServer::ActivatePlayerMoveEvent(int target, int player)
{
	WSAOVERLAPPEDEX* over_ex = new WSAOVERLAPPEDEX(OP::PLAYER_MOVE);
	over_ex->Target = player;
	gIOCP.PostToCompletionQueue(over_ex, target);
}

std::pair<short, short> IOCPServer::GetSectorIndex(int id)
{
	short x = gClients[id]->Info.x;
	short y = gClients[id]->Info.y;
	return { x / SECTOR_WIDTH, y / SECTOR_HEIGHT };
}

void IOCPServer::InsertIntoSectorWithoutLock(int id)
{
	auto secIdx = GetSectorIndex(id);
	gSectors[secIdx.first][secIdx.second].insert(id);
}

void IOCPServer::InsertIntoSectorWithLock(int id)
{
	auto sector = GetSectorIndex(id);

	mSectorLock.lock();
	gSectors[sector.first][sector.second].insert(id);
	mSectorLock.unlock();
}

void IOCPServer::EraseFromSectorWidthLock(int id)
{
	auto sector = GetSectorIndex(id);

	mSectorLock.lock();
	gSectors[sector.first][sector.second].erase(id);
	mSectorLock.unlock();
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