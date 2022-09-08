#pragma once

#include <stdafx.h>
#include <Socket.h>
#include <EndPoint.h>
#include <NetException.h>
#include <BufferQueue.h>
#include <Protocol.h>
#include <IOCP.h>
#include <Timer.h>

namespace Helper
{
	inline bool IsNPC(int id)
	{
		return (id >= NPC_ID_START && id <= NPC_ID_END);
	}

	inline bool IsNear(const PlayerInfo& a_info, const PlayerInfo& b_info)
	{
		if (ABS(a_info.x - b_info.x) > RANGE) return false;
		if (ABS(a_info.y - b_info.y) > RANGE) return false;
		return true;
	}

	inline void MovePosition(short& x, short& y, char direction)
	{
		switch (direction)
		{
		case 0: if (y > 0) y--; break;
		case 1: if (y < WORLD_HEIGHT - 1) y++; break;
		case 2: if (x > 0) x--; break;
		case 3: if (x < WORLD_WIDTH - 1) x++; break;
		}
	}
}